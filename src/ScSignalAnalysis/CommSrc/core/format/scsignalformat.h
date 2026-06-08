#ifndef SCSIGNALPARSER_H
#define SCSIGNALPARSER_H

#include "utils/scsaglobal.h"

class ScSignalFormat
{
public:
	~ScSignalFormat() = default;

	/**
	 * @brief type 当前数据格式类型
	 **/
	Sc::DataFormat type() const { return m_type; }

	/**
	 * @brief blockMode 获取数据块模式
	 **/
	Sc::DataBlockMode blockMode() const { return m_blockMode; }

	/**
	 * @brief bitDepth 量化比特数
	 **/
	Sc::BitDepth bitDepth() const { return m_bitDepth; }
	void setBitDepth(Sc::BitDepth depth) { m_bitDepth = depth; }
	void setBitDepth(unsigned short depth);

	/**
	 * @brief convertMode 获取字节转换模式
	 **/
	Sc::ConvertMode convertMode() const { return m_convertMode; }

	/**
	 * @brief sampleRate 采样率，单位Hz
	 **/
	double sampleRate() const { return m_sampleRate; }
	void setSampleRate(double rate) { m_sampleRate = rate; }

	/**
	 * @brief isIQ 输入数据是否是IQ格式
	 **/
	bool isIQ() const { return m_isIQ; }
	void setIQ(bool isIQ) { m_isIQ = isIQ; }

	/**
	 * @brief channelCount 通道数
	 */
	int channelCount() const { return m_channelCount; }
	void setChannelCount(int count) { m_channelCount = count; }

	/**
	 * @brief blockSize 数据块大小
	 */
	size_t blockSize() const { return m_blockSize; }

	/**
	 * @brief 获取每秒产生的字节数
	 * 计算公式：采样率 * 通道数 * (位深 / 8)
	 * note 复信号的通道数需要乘以2
	 */
	size_t byteRate() const {
		return static_cast<size_t>(m_sampleRate * m_channelCount * (static_cast<int>(m_bitDepth) / 8.0)) * (m_isIQ ? 2 : 1);
	}

	/**
	 * @brief calibrateMetaData 校准元数据
	 * @param metaData 待校准的元数据对象，校准后会更新数据格式、采样率、位深、通道数和数据类型等字段
	 */
	void calibrateMetaData(ScMetaData& metaData) const;

	/**
	 * @brief isValid 检查输入数据是否符合预期的格式
	 **/
	virtual bool isValid(const char* inBuff, int inlen) = 0;

	/**
	 * @brief headerLength 获取数据头长度，单位字节
	 **/
	virtual int headerLength() const = 0;

	/**
	 * @brief ratio 获取单字节可转出的字节数比例
	 * @return 转换比例（转出字节数 / 输入1字节）
	 */
	virtual double ratio() { return 1.0; }

private:
	Sc::DataFormat m_type;				/**< 格式类型 */
	Sc::DataBlockMode m_blockMode;		/**< 数据块模式 */
	Sc::BitDepth m_bitDepth;			/**< 量化比特数 */
	Sc::ConvertMode m_convertMode;		/**< 字节转换模式 */

protected:
	double m_sampleRate{ 0.0 };		/**< 采样率 */
	bool m_isIQ{ true };			/**< 是否是IQ格式 */
	int m_channelCount{ 1 };		/**< 通道数 */
	size_t m_blockSize{ 0 };		/**< 数据块大小 */

protected:
	explicit ScSignalFormat(Sc::DataFormat format, Sc::DataBlockMode blockMode = Sc::DBM_Multiple
		, Sc::ConvertMode convMode = Sc::CM_PerByte) : m_type(format), m_blockMode(blockMode)
		, m_bitDepth(Sc::BD_Unknown), m_convertMode(convMode) {
	}
};

inline void ScSignalFormat::setBitDepth(unsigned short depth)
{
	Sc::BitDepth bd = Sc::BD_Unknown;
	if (depth >= 1 && depth <= 64 && (0 == (depth & (depth - 1))))
		bd = static_cast<Sc::BitDepth>(depth);
	else
		fprintf(stderr, "[%s] An unsupported quantization bit number '%d' was used.", __FUNCTION__, depth);
	setBitDepth(bd);
}

inline void ScSignalFormat::calibrateMetaData(ScMetaData& metaData) const
{
	metaData.dataFormat = m_type;
	metaData.dataType = m_isIQ ? Sc::DT_IQ : Sc::DT_Real;
	if (m_sampleRate > 0)
		metaData.sampleRate = m_sampleRate;

	if (Sc::BD_Unknown != m_bitDepth)
		metaData.bitDepth = m_bitDepth;

	if (metaData.channelCount < m_channelCount)
		metaData.channelCount = m_channelCount;
}

template <typename Parser, typename Header>
struct ScParserCreator
{
	static ScSignalFormat* create() { return new Parser(); }
};

#endif // SCSIGNALPARSER_H
