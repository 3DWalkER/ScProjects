#include "scformatparser.h"

#include "format/scsignalformat.h"

int ScFormatParser::s_maxHeadLenth = 0;

class ScFormatParserHelper
{
public:
	static void clearParsers();
};

void ScFormatParserHelper::clearParsers()
{
	auto& parsers = ScFormatParser::parsers();
	for (auto it = parsers.begin(); it != parsers.end(); ++it)
		delete it->second;
}

static inline void clearParsers() {
	ScFormatParserHelper::clearParsers();
}
SC_DESTRUCTOR_FUNCTION(clearParsers)

ScFormatParser::ScFormatParser(Sc::DataFormat format)
	: m_formatType(Sc::DF_Unknown)
{

}

ScFormatParser::~ScFormatParser()
{
	
}

bool ScFormatParser::verifyHeader(const char* data, int inlen, int& verifyLength, int& blockSize)
{
	blockSize = 0;
	verifyLength = 0;
	if (Sc::DF_None == m_formatType)
		return true;

	if (!m_pFormat)
	{
		if (Sc::DF_Unknown == m_formatType)
			m_formatType = findFormatType(data, inlen);

		m_pFormat = parserFactory()[m_formatType]();
		if (!m_pFormat)
		{
			m_formatType = Sc::DF_None;
			return true;
		}
	}

	if (!m_pFormat->isValid(data, inlen))
		return false;

	blockSize = m_pFormat->blockSize();
	verifyLength = m_pFormat->headerLength();
	return true;
}

Sc::DataFormat ScFormatParser::findFormatType(const char* data, int inlen)
{
	ScSignalFormat* pFormat = nullptr;
	for (auto it = parsers().begin(); it != parsers().end(); ++it)
	{
		pFormat = it->second;
		if (pFormat->isValid(data, inlen))
			return it->first;
	}
	return Sc::DF_None;
}
