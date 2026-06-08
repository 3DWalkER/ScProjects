#ifndef SCSIGNALDATAMANAGER_H
#define SCSIGNALDATAMANAGER_H

#include "utils/scsaglobal.h"

/**
 * @brief The ScSignalDataManager class 数据管理器基类，定义数据读取的统一接口
 */
class ScSignalDataManager
{
	friend class ScConvertHelper;
public:
	virtual ~ScSignalDataManager();

	/**
	 * @brief source 数据来源
	 */
	Sc::DataSource source() const { return m_source; }

	/**
	 * @brief open 打开管理器
	 * @param mode			[in]数据读取模式，默认为连续读取
	 * @return true：打开成功，false：打开失败
	 */
	virtual bool open(Sc::ReadMode mode = Sc::RM_Sequential) = 0;

	/**
	 * @brief isOpen 设备管理器是否已经打开
	 * @return true：已经打开，false：未打开
	 */
	virtual bool isOpen() const = 0;

	/**
	 * @brief isAtEnd 是否在结尾
	 * @return true：在结尾，false：不在结尾
	 */
	virtual bool isAtEnd() const = 0;

	/**
	 * @brief seekSecond 定位到指定秒数的位置
	 * @param sec			[in]秒数
	 * @return true：定位成功，false：定位失败
	 */
	bool seekSecond(int sec);

	/**
	 * @brief seek 定位到指定字节数的位置
	 * @param offset		[in]偏移字节数
	 * @return true：定位成功，false：定位失败
	 */
	virtual bool seek(scuint64 offset) = 0;

	/**
	 * @brief read 读取数据到缓冲区，长度由length指定，数据类型由模板参数T指定
	 * @tparam T 数据类型，仅支持基本数据类型，如char、short、int、float、double等
	 */
	template <typename T>
	int read(T* buffer, int length) {
		static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported.");
		return read(sizeof(T), buffer, length);
	}

	/**
	 * @brief readRawChars 读取原始数据到缓冲区，长度由length指定
	 */
	virtual int readRawChars(char* buffer, int length) = 0;

	/**
	 * @brief write 向缓存中写入数据
	 * @param data			[in]要写入的数据
	 * @param isEnd			[in]是否为最后一次写入
	 */
	virtual int write(const char* data, int inlen, bool isEnd) = 0;

	/**
	 * @brief close 关闭设备管理器
	 */
	virtual void close() = 0;

private:
	/**
	 * @brief read 读取数据到缓冲区，长度由length指定
	 * @param byteSize 数据字节大小
	 * @param buffer 数据缓冲区
	 * @param length 数据长度
	 * @return 实际读取的数据长度
	 */
	int read(int byteSize, void* buffer, int length);

	Sc::DataSource m_source;				/**< 数据来源 */
	class ScConvertHelper* m_pConverter{ };	/**< 数据转换器，负责将原始字节数据转换为指定数值类型的数据 */

protected:
	usrid id;			/**< 数据标识 */
	int channel;		/**< 通道号 */
	int subChannel;		/**< 子通道号 */

protected:
	/**
	 * @brief CDSignalDataManager 构造函数，参数source指定数据来源
	 **/
	explicit ScSignalDataManager(Sc::DataSource source, const usrid& id = usrid(), int channel = 0, int subChannel = 0);
};


/**
 * @brief The ScFileDataManager class 文件数据管理器
 */
class ScFileDataManager : public ScSignalDataManager
{
public:
	bool open(Sc::ReadMode mode = Sc::RM_Sequential) override;
	bool isAtEnd() const override;
	bool seek(scuint64 offset) override;
	int readRawChars(char* buffer, int length) override;
	int write(const char* data, int inlen, bool isEnd) override;

	/**
	 * @brief fileSize 获取文件大小
	 */
	virtual scuint64 fileSize() const = 0;

private:
	scuint64 order{ };					/**< 文件序号 */
	bool isRealAtEnd{ };				/**< 是否真正的在结尾，某些数据源可能会有伪结尾的情况，比如文件列表中的最后一个文件已经读取完了，但整个数据源还没有结束 */
	mutable ScString nextFileName;		/**< 下一个文件的名称 */
	mutable Sc::HeaderStrategy hs{ };	/**< 数据头重新验证的策略，某些数据源可能会有数据头变化的情况 */

protected:
	ScString fileName;			/**< 文件名称 */
	Sc::ReadMode readMode{ };	/**< 数据读取模式 */

protected:
	explicit ScFileDataManager(Sc::DataSource source, const ScString& fileName, const usrid& id = usrid(), int channel = 0, int subChannel = 0);

	/**
	 * @brief openCurrentFile 打开当前文件
	 */
	virtual bool openCurrentFile() = 0;

	/**
	 * @brief isCurrFileEof 当前文件是否已经读取到结尾
	 */
	virtual bool isCurrFileEof() const = 0;

	/**
	 * @brief seekCurrentFile 在当前文件中定位到指定字节数的位置
	 */
	virtual bool seekCurrentFile(scuint64 offset) = 0;

	/**
	 * @brief readCurrentFile 读取当前文件的数据到缓冲区
	 */
	virtual int readCurrentFile(char* buffer, int length) = 0;
};


/**
 * The CDLocalDataManager class 文件数据管理器，从本地文件读取数据
 **/
class ScLocalDataManager : public ScFileDataManager
{
	SC_DISABLE_COPY_MOVE(ScLocalDataManager)
public:
	explicit ScLocalDataManager(const ScString& fileName, const usrid& id = ScString(), int channel = 0, int subChannel = 0);
	~ScLocalDataManager() override;

	bool isOpen() const override;
	scuint64 fileSize() const override;
	void close() override;

private:
	FILE* fp;

protected:
	bool openCurrentFile() override;
	bool isCurrFileEof() const override;
	bool seekCurrentFile(scuint64 offset) override;
	int readCurrentFile(char* buffer, int length) override;
};

#endif // SCSIGNALDATAMANAGER_H
