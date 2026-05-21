#include "scwaveformat.h"

ScWaveFormat::ScWaveFormat()
	: ScSignalFormat(Sc::DF_Wave, Sc::DBM_Single, Sc::CM_Normal)
{
}

bool ScWaveFormat::isValid(const char* inBuff, int inlen)
{
	m_blockSize = 0;
	if (!inBuff || inlen < sizeof(WAVE_HEADER))
		return false;

	const WAVE_HEADER* pHeader = reinterpret_cast<const WAVE_HEADER*>(inBuff);
	bool isOk = 0 == strncmp("RIFF", pHeader->ChunkID, 4) && 0 == strncmp("WAVE", pHeader->Format, 4)
		&& (0 == strncmp("fmt ", pHeader->Subchunk1ID, 4) || 0 == strncmp("fmt", pHeader->Subchunk1ID, 3))
		&& 0 == strncmp("data", pHeader->Subchunk2ID, 4);

	if (isOk)
	{
		m_sampleRate = static_cast<double>(pHeader->SampleRate);
		m_isIQ = (0x2 == pHeader->NumChannels);
		m_channelCount = 1;
		m_blockSize = pHeader->Subchunk2Size;
		setBitDepth(pHeader->BitsPerSample);
	}
	return isOk;
}

double ScWaveFormat::ratio()
{
	const int bd = static_cast<int>(bitDepth());
	return bd <= 0 ? 0.5 : (8.0 / bd);
}