#include "scvsiformat.h"

#include "scutils/utils/scprocessordetection.h"

ScVsiFormat::ScVsiFormat()
	: ScSignalFormat(Sc::DF_VSI, Sc::DBM_Multiple, Sc::CM_PerByte)
{
}

bool ScVsiFormat::isValid(const char* inBuff, int inlen)
{
	m_blockSize = 0;
	if (!inBuff || inlen < sizeof(VSI_HEADER))
		return false;

	const VSI_HEADER* pHeader = reinterpret_cast<const VSI_HEADER*>(inBuff);
#if SC_LITTLE_ENDIAN == SC_BYTE_ORDER
	bool isOk = 0b10101011101011011101111011101101 == pHeader->record_label;
#else
	bool isOk = return 0b11011110111011011101101010111010 == pHeader->record_label;
#endif

	if (isOk)
	{
		m_blockSize = 10000;
		m_isIQ = false;
		setBitDepth(Sc::BD_Unknown);
	}
	return isOk;
}
