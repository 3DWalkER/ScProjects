#include "scchannelreader.h"

#include "scutils/thread/screadwritelock.h"
#include "scobjecpool.h"

/**
 * @brief s_cacheLength 缓存数据大小
 */
static size_t s_cacheLength = LEN128M;

/**
 * @brief 带有引用计数的索引包装器
 */
struct ScEndIndex
{
	ScAtomicInt ref;	/**< 原子引用计数，用于控制当前索引项的安全释放与弹出 **/
	size_t index{ 0 };	/**< 存储的实际业务索引值或位置 **/
	int count{ 0 };		/**< 内部基准计数（用于与期望的引用计数做差值校准） **/
};

class ScSubChannel;

/**
 * The ScChannelData class 通道数据结构
 */
class ScChannelData : public ScObject
{
public:
	ScChannelData();
	~ScChannelData() override;

	/**
	 * @brief 重置通道数据、游标及结束标记队列
	 */
	void reset() override;

	/**
	 * @brief 在当前写入位置创建一个结束索引标记并压入队列
	 * @param index 当前的结束位置
	 */
	void pushEndIndex(int index);

	/**
	 * @brief checkAndSyncEndIdx 检查当前读取范围是否触及通道结束标记，并在触及时校准引用计数
	 * @param readLen       [in]当前步骤原本计划读取的数据长度
	 * @param currReadIdx	[in]当前步骤读取的起始游标位置（第一段为 readIdx，第二段翻转后为 0）
	 * @param isAtEnd       [ou]用于标记发起读取的子通道的isAtEnd状态指针
	 * @return int 经过边界检查和截断后，实际允许读取的安全数据长度
	 */
	int checkAndSyncEndIdx(int readLen, int currReadIdx, bool& isAtEnd);

	/**
	 * @brief syncEndIdxRefAndDeref 同步引用计数并尝试释放/弹出索引
	 * @param endIdx 需要被校准和deref的目标对象
	 */
	void syncEndIdxRefAndDeref(ScEndIndex& endIdx);

	/**
	 * @brief 获取子通道未读取数据的最大值
	 */
	size_t maxUsedSpace();

	/**
	 * @brief 尝试在没有真实连续通道时，推选一个非连续通道作为代班门控
	 * @note 调用此函数前，外部必须已经持有subChMutex锁
	 */
	void tryPromoteNewGatingChannel_Locked();

	/**
	 * @brief 动态遍历所有子通道，获取当前最慢的受保护读取字节数
	 * @param curWrite 当前生产者的绝对写入总量
	 * @return scuint64 当前全局最慢的读取字节数
	 * @note 调用此函数前，外部必须已持有spaceWaitMutex锁
	 */
	size_t slowestReadBytes_Locked(size_t curWrite);

	char* m_pCacheBuff{ };		/**< 缓存数据缓冲区 */
	ScAtomicInteger<size_t> totalWriteBytes{ 0 };	/** 本通道累计写入字节数 */
	size_t writeIdx{ };         /**< 环形缓冲区的当前写入游标位置 **/
	ScReadWriteLock rwlock;     /**< 保护整个通道数据读写的主读写锁 */

	ScAtomicInt seqSubChCount;				/**< 活跃的顺序读取子通道原子计数 */
	std::vector<ScSubChannel*> subChannels;	/**< 子通道 */
	std::mutex subChMutex;					/**< 子通道操作使用的互斥锁 */

	std::mutex spaceWaitMutex;				/**< 空闲空间等待互斥锁 */
	std::condition_variable spaceWaitCv;	/**< 空闲空间等待条件变量 */
	std::atomic<bool> isTerminated{ false };

	bool isAtEnd{ };							/**< 通道是否已到达末尾的标志 */
	std::mutex dataAvailableLock;				/**< 保护数据可用条件变量的互斥锁 */
	std::condition_variable dataAvailableCv;	/**< 数据可用条件变量，用于通知等待数据的消费者线程 */

	std::queue<ScEndIndex> endIdxes;    /**< 结束索引标记队列，用于流式截断和生命周期感知 */
	std::mutex endIdxesLock;            /**< 保护endIdxes队列并发安全的互斥锁 */

private:
};

/**
 * The ScChannelPool class 通道数据池
 */
using ScChannelPool = ScRecyclablePool<ScChannelData>;

/**
 * The ScSubChannel class 子通道数据结构，包含子通道号
 */
class ScSubChannel
{
public:
	ScSubChannel(int channel, Sc::ReadMode mode);
	~ScSubChannel();

	/**
	 * @brief 重置索引
	 */
	void reset();

	size_t read(ScChannelData* d, char* pBuff, const size_t readlen);
	size_t read_sequential(ScChannelData* d, char* pBuff, const size_t readlen);
	size_t read_discreate(ScChannelData* d, char* pBuff, const size_t readlen);

	int channel;			/**< 通道号 **/
	Sc::ReadMode mode;		/**< 读取模式 **/

	ScAtomicInteger<bool> isOpen;				/**< 该通道是否已经打开 */
	ScAtomicInteger<bool> isGating{ false };	/**< true代表处于代班或原生连续读取状态 **/

	size_t readIdx{ 0 };		/**< 通道当前读取位置 **/
	size_t totalReadBytes{ 0 };	/**< 通道累计读取字节数 **/
	bool isAtEnd{ };			/**< 是否已经读取结束 **/
};

ScChannelData::ScChannelData()
	: m_pCacheBuff(new char[s_cacheLength])
{
}

ScChannelData::~ScChannelData()
{
	SC_SAVE_DELETE(m_pCacheBuff);
}

void ScChannelData::reset()
{
	isAtEnd = false;
	totalWriteBytes = 0;
	writeIdx = 0;

	std::lock_guard<std::mutex> locker(endIdxesLock);
	endIdxes = std::queue<ScEndIndex>();
}

void ScChannelData::pushEndIndex(int index)
{
	ScEndIndex endIdx;
	endIdx.count = ref.loadRelaxed();
	endIdx.ref = endIdx.count;
	endIdx.index = index;

	std::lock_guard<std::mutex> locker(endIdxesLock);
	endIdxes.push(std::move(endIdx));
}

int ScChannelData::checkAndSyncEndIdx(int readLen, int currReadIdx, bool& isAtEnd)
{
	isAtEnd = false;
	if (endIdxes.empty())
		return readLen;

	std::lock_guard<std::mutex> locker(endIdxesLock);
	if (endIdxes.empty())
		return readLen;

	auto& endIdx = endIdxes.front();
	if (endIdx.index <= currReadIdx + readLen && endIdx.index >= currReadIdx)
	{
		isAtEnd = true;
		readLen = endIdx.index - currReadIdx;
		syncEndIdxRefAndDeref(endIdx);
	}
	return readLen;
}

void ScChannelData::syncEndIdxRefAndDeref(ScEndIndex& endIdx)
{
	const int delta = ref.loadRelaxed() - endIdx.count;
	if (0 != delta)
		endIdx.ref.fetchAndAddRelaxed(delta);

	if (!endIdx.ref.deref())
		endIdxes.pop();
}

size_t ScChannelData::maxUsedSpace()
{
	std::lock_guard<std::mutex> locker(spaceWaitMutex);
	const size_t curWrite = totalWriteBytes.loadAcquire();
	const size_t slowestRead = slowestReadBytes_Locked(curWrite);
	if (curWrite <= slowestRead)
		return 0;

	const size_t used = curWrite - slowestRead;
	if (used >= static_cast<scuint64>(s_cacheLength))
		return s_cacheLength;
	return used;
}

void ScChannelData::tryPromoteNewGatingChannel_Locked()
{
	if (seqSubChCount.loadAcquire() > 0)
		return;

	for (auto* ch : subChannels)
	{
		if (Sc::RM_Discrete == ch->mode)
		{
			ch->isGating.storeRelease(true);
			break;
		}
	}
}

size_t ScChannelData::slowestReadBytes_Locked(size_t curWrite)
{
	std::lock_guard<std::mutex> lock(subChMutex);
	if (subChannels.empty())
		return curWrite;

	scuint64 slowestRead = curWrite;
	bool hasGatingConsumer = false;
	for (const auto* ch : subChannels)
	{
		if (1 == ch->isGating.loadRelaxed())
		{
			hasGatingConsumer = true;
			if (ch->totalReadBytes < slowestRead)
				slowestRead = ch->totalReadBytes;
		}
	}
	return hasGatingConsumer ? slowestRead : curWrite;
}

ScSubChannel::ScSubChannel(int channel, Sc::ReadMode mode)
	: channel(channel)
	, mode(mode)
{
}

ScSubChannel::~ScSubChannel()
{
}

void ScSubChannel::reset()
{
	readIdx = 0;
	totalReadBytes = 0;
	isAtEnd = false;
}

size_t ScSubChannel::read(ScChannelData* d, char* pBuff, const size_t readlen)
{
	if (isAtEnd)
		return 0;
	const size_t currWriteBytes = d->totalWriteBytes.loadAcquire();
	const size_t cacheSize = currWriteBytes - totalReadBytes;
	const size_t diff = s_cacheLength - readIdx;
	size_t length = scMin(scMin(diff, cacheSize), readlen);
	length = d->checkAndSyncEndIdx(length, readIdx, isAtEnd);
	if (length > 0)
		memcpy(pBuff, d->m_pCacheBuff + readIdx, length * sizeof(char));

	readIdx += length;
	if (isAtEnd || readlen == length || diff > cacheSize)
		return length;

	int remLen = scMin(cacheSize - length, readlen - length);
	remLen = d->checkAndSyncEndIdx(remLen, 0, isAtEnd);
	if (remLen > 0)
		memcpy(pBuff + length, d->m_pCacheBuff, remLen * sizeof(char));
	readIdx = remLen;
	return length + remLen;
}

size_t ScSubChannel::read_sequential(ScChannelData* d, char* pBuff, const size_t readlen)
{
	int length = read(d, pBuff, readlen);
	totalReadBytes += length;
	return length;
}

size_t ScSubChannel::read_discreate(ScChannelData* d, char* pBuff, const size_t readlen)
{
	const size_t cacheSize = d->maxUsedSpace();
	size_t length = scMin(cacheSize, readlen);
	if (length <= 0)
		return 0;

	readIdx = (d->writeIdx - cacheSize + s_cacheLength) % s_cacheLength;
	return read(d, pBuff, length);
}

ScChannelReader::ScChannelReader(const usrid& id, int channel, int subChannel, bool isExclusive)
	: ch(new ScSubChannel(subChannel, Sc::RM_Sequential))
	, d(ScChannelPool::acquire(id, channel, subChannel, isExclusive))
{
}

ScChannelReader::~ScChannelReader()
{
	close();
	if (d)
		ScChannelPool::release(d);
	delete ch;
}

bool ScChannelReader::open(Sc::ReadMode mode)
{
	if (!ch || !d)
		return false;

	if (!ch->isOpen.testAndSetRelaxed(false, true))
		return true;

	ch->mode = mode;
	ch->reset();

	std::lock_guard<std::mutex> lock(d->subChMutex);
	if (Sc::RM_Sequential == mode)
	{
		d->seqSubChCount.ref();
		ch->isGating.storeRelease(true);
		for (auto* ch : d->subChannels)
		{
			if (Sc::RM_Discrete == ch->mode)
				ch->isGating.storeRelease(false);
		}
	}
	else
	{
		if (d->seqSubChCount.loadAcquire() <= 0)
		{
			bool alreadyHasGating = false;
			for (auto* ch : d->subChannels)
			{
				if (1 == ch->isGating.loadRelaxed())
				{
					alreadyHasGating = true;
					break;
				}
			}

			if (!alreadyHasGating)
				ch->isGating.storeRelease(true);
		}
	}

	d->subChannels.push_back(ch);
	return true;
}

bool ScChannelReader::isOpen() const
{
	return ch && ch->isOpen.loadAcquire();
}

bool ScChannelReader::isAtEnd() const
{
	if (!ch || !d)
		return true;
	return ch->isAtEnd && ch->totalReadBytes >= d->totalWriteBytes.loadAcquire();

}

bool ScChannelReader::isWriteAtEnd() const
{
	if (!d)
		return false;

	ScReadLocker locker(&d->rwlock);
	return d->isAtEnd;
}

size_t ScChannelReader::read(char* pBuff, const size_t readlen)
{
	if (!d || !pBuff || readlen <= 0)
		return 0;

	char* pCurrBuff = pBuff;
	int progress = 0, batchRead = 0;
	while (progress < readlen && !ch->isAtEnd)
	{
		const size_t currWriteBytes = d->totalWriteBytes.loadAcquire();
		const size_t cacheSize = currWriteBytes - ch->totalReadBytes;
		const int remainRead = readlen - progress;
		const int batchReadLen = std::min({ static_cast<size_t>(remainRead), cacheSize, s_cacheLength });
		if (batchReadLen < remainRead)
		{
			std::unique_lock<std::mutex> locker(d->dataAvailableLock);
			d->dataAvailableCv.wait(locker, [&]() {
				const size_t newWriteBytes = d->totalWriteBytes.loadAcquire();
				const size_t newCacheSize = newWriteBytes - ch->totalReadBytes;
				return newCacheSize >= std::min(static_cast<size_t>(remainRead), s_cacheLength)
					|| d->isAtEnd;
			});
		}

		{
			ScReadLocker locker(&d->rwlock);
			if (Sc::RM_Sequential != ch->mode)
			{
				if (ch->isGating.loadAcquire())
					batchRead = ch->read_sequential(d, pCurrBuff, std::min(static_cast<size_t>(remainRead), s_cacheLength));
				else
					batchRead = ch->read_discreate(d, pCurrBuff, std::min(static_cast<size_t>(remainRead), s_cacheLength));
			}
			else
				batchRead = ch->read_sequential(d, pCurrBuff, std::min(static_cast<size_t>(remainRead), s_cacheLength));
		}

		progress += batchRead;
		pCurrBuff += batchRead;
		if (s_cacheLength == ch->readIdx)
			ch->readIdx = 0;

		d->spaceWaitCv.notify_one();
	}
	return progress;
}

size_t ScChannelReader::write(const char* pBuff, const size_t inlen, bool isEnd)
{
	if (!d || !pBuff || inlen <= 0)
		return 0;

	if (!waitForFreeSpace(inlen, -1))
		return 0;

	{
		ScWriteLocker locker(&d->rwlock);
		const int diff = s_cacheLength - d->writeIdx;
		if (inlen >= diff)
		{
			if (0 != diff)
				memcpy(d->m_pCacheBuff + d->writeIdx, pBuff, diff * sizeof(char));
			d->writeIdx = inlen - diff;
			SC_ASSERT(d->writeIdx <= s_cacheLength);
			memcpy(d->m_pCacheBuff, pBuff + diff, d->writeIdx * sizeof(char));
		}
		else
		{
			memcpy(d->m_pCacheBuff + d->writeIdx, pBuff, inlen * sizeof(char));
			d->writeIdx += inlen;
		}

		if (isEnd)
		{
			d->isAtEnd = true;
			d->pushEndIndex(d->writeIdx);
		}
		d->totalWriteBytes.fetchAndAddRelease(inlen);
	}
	d->dataAvailableCv.notify_all();
	return inlen;
}

bool ScChannelReader::waitForFreeSpace(int requiredLen, int timeoutMs)
{
	if (!d)
		return false;

	auto isSpaceAvailable = [&]() {
		if (d->isTerminated)
			return true;

		const size_t curWrite = d->totalWriteBytes.loadAcquire();
		const size_t slowestRead = d->slowestReadBytes_Locked(curWrite);
		const size_t usedSpace = curWrite - slowestRead;
		return (s_cacheLength - usedSpace) >= static_cast<uint64_t>(requiredLen);
	};

	std::unique_lock<std::mutex> locker(d->spaceWaitMutex);
	bool isOk = false;
	if (timeoutMs < 0)
	{
		d->spaceWaitCv.wait(locker, isSpaceAvailable);
		isOk = true;
	}
	else
		isOk = d->spaceWaitCv.wait_for(locker, std::chrono::milliseconds(timeoutMs), isSpaceAvailable);

	if (d->isTerminated)
	{
		d->isTerminated = false;
		return false;
	}
	return isOk;
}

void ScChannelReader::interruptWait()
{
	if (!d)
		return;

	std::unique_lock<std::mutex> locker(d->spaceWaitMutex);
	d->isTerminated = true;
	d->spaceWaitCv.notify_all();
}

void ScChannelReader::reset()
{
	if (d)
	{
		ScWriteLocker locker(&d->rwlock);
		ch->reset();
		d->reset();
	}
}

void ScChannelReader::close()
{
	if (!ch || !d)
		return;

	if (!ch->isOpen.testAndSetRelaxed(true, false))
		return;

	std::lock_guard<std::mutex> lock(d->subChMutex);
	auto it = std::remove(d->subChannels.begin(), d->subChannels.end(), ch);
	if (it != d->subChannels.end())
		d->subChannels.erase(it, d->subChannels.end());

	if (Sc::RM_Sequential == ch->mode)
	{
		if (!d->seqSubChCount.deref())
			d->tryPromoteNewGatingChannel_Locked();
	}
	else
	{
		if (ch->isGating.loadRelaxed())
		{
			ch->isGating.storeRelease(false);
			d->tryPromoteNewGatingChannel_Locked();
		}
	}
	d->spaceWaitCv.notify_all();
}