#ifndef SCOBJECTPOOL_H
#define SCOBJECTPOOL_H

#include "scutils/thread/screadwritelock.h"
#include "sckey.h"
#include "scsaglobal.h"

#include <unordered_map>
#include <queue>
#include <mutex>

/**
 * @brief 最大并行分片数量（必须为2的幂以优化取模运算）
 */
static constexpr int kMaxParallelism = 64;
static_assert(0 == (kMaxParallelism & (kMaxParallelism - 1)), "kMaxParallelism must be a power of 2");

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

template <typename T>
struct is_unique_ptr : std::false_type {};

template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE

/**
 * @brief ScObject 池化对象基类
 */
class ScObject
{
public:
	usrid id;					/**< 用户ID，标识对象所有者 */
	int channel{ 0 };			/**< 通道号，标识通道类型 */
	size_t proxyChannel{ 0 };	/**< 代理通道号，用于代理模式 */
	ScAtomicInt ref{ 0 };		/**< 原子引用计数，初始化为0，用于生命周期管理 */

	ScObject() = default;
	ScObject(const usrid& id, int channel) : id(id), channel(channel) {}
	virtual ~ScObject() = default;

	/**
	 * @brief 重置对象状态（派生类需重写）
	 *
	 * 使用场景：
	 *   - 对象从池中取出前，需要清空内部状态
	 *   - 调用此方法而非析构函数，支持对象复用
	 */
	virtual void reset() {};
};


/**
 * @brief ScObjectPool 模板化的对象池基类，实现桶式存储结构
 *        每个桶有独立的读写锁，减少锁竞争
 */
template <typename T>
class ScObjectPool
{
protected:
	using ScObjects = std::unordered_map<ScUsrKey, T, ScUsrKeyHasher>;

	/**
	 * @brief 分片桶结构
	 *
	 * 职责：
	 *   - rwlock：读写锁，保护 objects 容器的并发访问
	 *   - objects：存储该分片的所有对象
	 *
	 * 设计理由：
	 *   - 禁止拷贝/移动：避免锁对象被非法复制导致同步失效
	 */
	struct Bucket
	{
		ScReadWriteLock rwlock;		/**< 读写锁：支持多个读者或单个写者 */
		ScObjects objects;			/**< 对象容器 */

		Bucket() = default;
		Bucket(const Bucket&) = delete;
		Bucket& operator=(const Bucket&) = delete;
		Bucket(Bucket&&) = delete;
		Bucket& operator=(Bucket&&) = delete;
	};

	/**
	 * @brief 根据 key 计算所属的分片桶
	 *
	 * 算法：
	 *   hash_value % kMaxParallelism
	 *   优化为：hash_value & (kMaxParallelism - 1)
	 *   原因：& 运算比 % 快，前提是 kMaxParallelism 是 2 的幂
	 *
	 * @param key			[in]对象 key（包含 user_id 和 channel）
	 * @return 该 key 对应的桶引用
	 */
	static inline Bucket& bucket(const ScUsrKey& key) {
		const size_t h = ScUsrKeyHasher{}(key);
		return buckets[h & (kMaxParallelism - 1)];
	}

	/**
	 * @brief 便捷重载：根据 user_id 和 channel 获取分片桶
	 * @param id 用户 ID
	 * @param channel 通道号
	 * @return 分片桶引用
	 */
	static inline Bucket& bucket(const usrid& id, size_t channel) { return bucket({ id, channel }); }

	/**
	 * @brief 通用获取逻辑：从对象容器中查找并克隆对象
	 * @param objs			[in]对象容器
	 * @param key			[in]查找 key
	 * @return 克隆后的对象或默认值
	 */
	template <typename U = T, typename std::enable_if<!SC_DETAIL::is_unique_ptr<U>::value, int>::type = 0>
	static T tryAcquire(ScObjects& objs, const ScUsrKey& key);

	/**
	 * @brief tryAcquire 重载函数，仅支持std::unique_ptr类型
	 */
	template <typename U = T, typename std::enable_if<SC_DETAIL::is_unique_ptr<U>::value, int>::type = 0>
	static T tryAcquire(ScObjects& objs, const ScUsrKey& key);

private:
	static Bucket buckets[kMaxParallelism];		/**< 静态存储的分片桶数组 */
};

template <typename T>
typename ScObjectPool<T>::Bucket ScObjectPool<T>::buckets[kMaxParallelism]{};


/**
 * @brief ScExternalPool 开放式对象池
 * 特点：外部创建、投放对象，不拥有对象所有权，不负责创建与销毁
 */
template <typename T>
class ScExternalPool : public ScObjectPool<T>
{
public:
	/**
	 * @brief tryAcquire 通用获取接口：从对象池中获取对象
	 * @param id			[in]用户ID
	 * @param channel		[in]通道号
	 * @return 克隆后的对象副本，若对象不存在则返回默认值
	 */
	static T tryAcquire(const usrid& id, size_t channel);

	/**
	 * @brief emplace 投放接口（通用版）：将对象投放到对象池
	 *
	 * 支持类型：
	 *   - 栈对象（值类型）
	 *   - 普通指针
	 *   - std::shared_ptr
	 *
	 * @param id			[in]用户ID
	 * @param channel		[in]通道号
	 * @return true: 投放成功；false: 对象已存在，投放失败
	 */
	template <typename U = T, typename std::enable_if<!SC_DETAIL::is_unique_ptr<U>::value, int>::type = 0>
	bool emplace(const usrid& id, size_t channel, T& obj);

	/**
	 * @brief emplace 重载函数，仅支持std::unique_ptr类型
	 */
	template <typename U = T, typename std::enable_if<SC_DETAIL::is_unique_ptr<U>::value, int>::type = 0>
	static bool emplace(const usrid& id, size_t channel, T&& obj);

	/**
	 * @brief release 释放接口：从对象池中删除但不销毁对象
	 * @param id			[in]用户ID
	 * @param channel		[in]通道号
	 * @return 释放的对象
	 */
	static T release(const usrid& id, size_t channel);
};

template<typename T>
template <typename U, typename std::enable_if<!SC_DETAIL::is_unique_ptr<U>::value, int>::type>
inline T ScObjectPool<T>::tryAcquire(ScObjects& objs, const ScUsrKey& key)
{
	auto it = objs.find(key);
	if (it != objs.end())
		return it->second;
	return {};
}

template<typename T>
template <typename U, typename std::enable_if<SC_DETAIL::is_unique_ptr<U>::value, int>::type>
inline T ScObjectPool<T>::tryAcquire(ScObjects& objs, const ScUsrKey& key)
{
	auto it = objs.find(key);
	if (it != objs.end())
	{
		auto ptr = std::move(it->second);
		objs.erase(it);
		return ptr;
	}
	return {};
}

template<typename T>
inline T ScExternalPool<T>::tryAcquire(const usrid& id, size_t channel)
{
	ScUsrKey key{ id, channel };
	auto& b = bucket(key);
	ScReadLocker locker(&b.rwlock);
	return ScObjectPool<T>::tryAcquire(b.objects, key);
}

template<typename T>
template <typename U, typename std::enable_if<!SC_DETAIL::is_unique_ptr<U>::value, int>::type>
bool ScExternalPool<T>::emplace(const usrid& id, size_t channel, T& obj)
{
	ScUsrKey key{ id, channel };
	auto& b = bucket(key);
	ScWriteLocker locker(&b.rwlock);
	auto it = b.objects.find(key);
	if (it != b.objects.end())
		return false;

	b.objects[key] = obj;
	return true;
}

template<typename T>
template <typename U, typename std::enable_if<SC_DETAIL::is_unique_ptr<U>::value, int>::type>
bool ScExternalPool<T>::emplace(const usrid& id, size_t channel, T&& obj)
{
	ScUsrKey key{ id, channel };
	auto& b = bucket(key);
	ScWriteLocker locker(&b.rwlock);
	auto it = b.objects.find(key);
	if (it != b.objects.end())
		return false;

	b.objects.emplace(key, std::move(obj));
	return true;
}

template<typename T>
inline T ScExternalPool<T>::release(const usrid& id, size_t channel)
{
	ScUsrKey key{ id, channel };
	auto& b = bucket(key);
	ScWriteLocker locker(&b.rwlock);
	auto it = b.objects.find(key);
	if (b.objects.end() == it)
		return T{};

	T ret = std::move(it->second);
	b.objects.erase(it);
	return ret;
}

#endif // SCOBJECTPOOL_H
