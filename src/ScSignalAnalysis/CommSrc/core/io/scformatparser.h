#ifndef SCFORMATPARSER_H
#define SCFORMATPARSER_H

#include "utils/scsaglobal.h"

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

private:
	Sc::DataFormat m_dataFormat;			/**< 数据格式类型 */
	ScSignalFormat* m_pFormat{ nullptr };	/**< 数据格式 */
};

#endif // SCFORMATPARSER_H
