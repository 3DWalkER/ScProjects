#ifndef SCFORMATPARSER_H
#define SCFORMATPARSER_H

#include "scsignalformat.h"
#include <unordered_map>

class ScSignalFormat;

/**
 * @brief The ScFormatParser class 字节转换器，负责将输入字节数据转换为指定数值类型的数据
 * @note 仅支持转换为char, unsigned char, short, int, float, double等基础数据类型
 * @note 内部不处理字节序，输入数据的字节序必须与目标类型字节序保持一致
 * @note 不做边界安全校验：输入字节长度不足、内存缓冲区重叠等问题由调用方保证
 */
class ScFormatParser
{
	friend class ScFormatParserHelper;
public:
	explicit ScFormatParser(Sc::DataFormat format = Sc::DF_Unknown);
	~ScFormatParser();

	/**
	 * @brief verifyLength 验证数据头所需要的数据长度
	 */
	int verifyLength() const { return verifySize > 0 ? verifySize : s_maxHeadLenth; }

	/**
	 * @brief verify 验证数据头
	 * @param data				[in]要验证的数据
	 * @param inlen				[in]要验证的数据长度
	 * @param verifyLength		[in]验证实际使用的数据长度
	 * @param blockSize			[in]有效字节长度
	 * @return true：验证成功，false：验证失败
	 */
	bool verifyHeader(const char* data, int inlen, int& verifyLength, int& blockSize);

	/**
	 * @brief registerFormat 注册数据格式解析器
	 *		@tparam Parser 解析器类型，必须继承自ScSignalFormat
	 *		@tparam Header 解析器对应的数据头结构类型
	 * @param format			[in]数据格式
	 */
	template <typename Parser, typename Header>
	static void registerFormat(Sc::DataFormat format);

private:
	Sc::DataFormat m_formatType;			/**< 数据格式类型 */
	int verifySize{ -1 };					/** 验证数据头所需的字节长度 **/
	ScSignalFormat* m_pFormat{ nullptr };	/**< 数据格式 */

	static int s_maxHeadLenth;		/** 已注册的解析器中最大数据头长度 **/

private:
	static Sc::DataFormat findFormatType(const char* data, int inlen);

	static std::unordered_map<Sc::DataFormat, ScSignalFormat*>& parsers();
	static std::unordered_map<Sc::DataFormat, ScSignalFormat* (*)()>& parserFactory();
};

template<typename Parser, typename Header>
inline void ScFormatParser::registerFormat(Sc::DataFormat format)
{
	auto it = parserFactory().find(format);
	if (it != parserFactory().end())
	{
		fprintf(stderr, "%s Data format (%d) already registered!\n", __FUNCTION__, format);
		return;
	}

	s_maxHeadLenth = scMax(static_cast<int>(sizeof(Header)), s_maxHeadLenth);
	auto& pCreator = ScParserCreator<Parser, Header>::create;
	parserFactory()[format] = pCreator;
	parsers()[format] = pCreator();
}

inline std::unordered_map<Sc::DataFormat, ScSignalFormat*>& ScFormatParser::parsers()
{
	static std::unordered_map<Sc::DataFormat, ScSignalFormat*> s_parsers;
	return s_parsers;
}

inline std::unordered_map<Sc::DataFormat, ScSignalFormat* (*)()>& ScFormatParser::parserFactory()
{
	static std::unordered_map<Sc::DataFormat, ScSignalFormat* (*)()> s_parserFactory;
	return s_parserFactory;
}

#endif // SCFORMATPARSER_H
