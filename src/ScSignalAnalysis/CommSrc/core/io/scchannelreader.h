#ifndef SCCHANNELREADER_H
#define SCCHANNELREADER_H

#include "scsaglobal.h"
#include <memory>

/**
 * The ScChannelReader class 通道读取器，负责从指定通道读取数据
 * 该类的实例化对象可以用于读取不同通道的数据，支持多通道和子通道的概念
 **/
class ScChannelReader
{
	SC_DISABLE_COPY_MOVE(ScChannelReader)
public:
	/**
	 * @brief create 析构函数
	 */
	~ScChannelReader();

	/**
	* @brief create 创建通道数据读取器
	 */
	static std::shared_ptr<ScChannelReader> create(const usrid& id, int channel, int subChannel = 0, bool isExclusive = false) {
		return std::shared_ptr<ScChannelReader>(new ScChannelReader(id, channel, subChannel, isExclusive));
	}

	/**
	 * @brief open 打开通道数据读取器
	 * @param mode			[in]读取模式，默认为Sc::RM_Sequential，表示顺序读取
	 */
	bool open(Sc::ReadMode mode = Sc::RM_Sequential);

	/**
	 * @brief isOpen 判断通道数据读取器是否已经打开
	 */
	bool isOpen() const;

	/**
	 * @brief isAtEnd 检查当前子通道是否已经读取到流的末尾
	 * @return true: 通道已到达末尾，无更多数据可读，false: 通道未结束，仍可继续尝试读取
	 */
	bool isAtEnd() const;

	/**
	 * @brief isWriteAtEnd 检查通道channel是否已处于“写入结束”状态
	 * @return true 写入已完成，无更多数据；false 仍处于活跃状态，可继续操作
	 */
	bool isWriteAtEnd() const;

	/**
	 * @brief read 从指定缓冲区读取数据
	 * @param pBuff			[in]源数据缓冲区的指针
	 * @param readlen		[in]请求读取的数据长度（字节数）
	 * @return int 成功读取的字节数
	 */
	size_t read(char* pBuff, const size_t readlen);

	/**
	 * @brief write 向通道的环形缓冲区写入数据，并可选地标记数据流结束
	 * @param pBuff			[in]待写入的源数据缓冲区指针
	 * @param inlen			[in]本次期望写入的数据字节长度
	 * @param isEnd			[in]是否为本次数据流的最后一批数据（置为 true 将触发结束索引标记）
	 * @return int 实际成功写入缓冲区的字节长度
	 */
	size_t write(const char* pBuff, const size_t inlen, bool isEnd);

	/**
	 * @brief waitForFreeSpace 阻塞等待，直到缓冲区空闲空间大于等于指定的长度，或发生超时
	 * @param requiredLen	[in]期望的最小空闲空间长度
	 * @param timeoutMs		[in]最大等待超时时间（毫秒）,小于0则一直等待
	 * @return true			[in]空间已充足，可以写入
	 * @return false		[in]超时或通道已关闭
	 */
	bool waitForFreeSpace(int requiredLen, int timeoutMs);

	/**
	 * @brief interruptWait 主动终止等待
	 */
	void interruptWait();

	/**
	 * @brief reset 重置通道数据读取器
	 */
	void reset();

	/**
	 * @brief close 关闭通道数据读取器
	 */
	void close();

private:
	/**
	 * @brief CDChannelReader 构造函数，指定通道号和子通道号
	 * @param id				[in]认证令牌，用于验证读取权限
	 * @param channel			[in]通道号，标识要读取的通道
	 * @param subChannel		[in]子通道号，标识要读取的子通道，默认为0
	 * @param isExclusive		[in]是否独占通道数据，即通道数据不共享
	 */
	explicit ScChannelReader(const usrid& id, int channel, int subChannel = 0, bool isExclusive = false);

	class ScSubChannel* ch;		/**< 子通道信息 */
	class ScChannelData* d;		/**< 通道数据 */

};

using ScChannelReaderPtr = std::shared_ptr<ScChannelReader>;

#endif // SCCHANNELREADER_H
