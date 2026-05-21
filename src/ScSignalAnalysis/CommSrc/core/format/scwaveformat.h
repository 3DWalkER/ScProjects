#ifndef SCWAVEFORMAT_H
#define SCWAVEFORMAT_H

#include "scsignalformat.h"

#pragma pack(1)  

typedef struct WAVE_HEADER
{
	char ChunkID[4];                /**< 固定模式 "RIFF" */
	unsigned int ChunkSize;         /**< 不包含开始 8 个字节 */
	char Format[4];                 /**< 固定为 "WAVE" */
	char Subchunk1ID[4];            /**< 固定为 "fmt " */
	unsigned int Subchunk1Size;     /**< 子块 1 大小，通常为 16 */
	unsigned short AudioFormat;     /**< 编码格式，即压缩格式，0x1: PCM(无压缩), 0x3: float, 0x0006, 0x0007, 0xFFFF */
	unsigned short NumChannels;     /**< 通道数，0x01: 单声道, 0x02: 双声道 */
	unsigned int SampleRate;        /**< 采样率 (Hz) */
	unsigned int ByteRate;          /**< 每秒存储的字节数，SampleRate * BlockAlign */
	unsigned short BlockAlign;      /**< 块对齐大小，NumChannels * BitsPerSample / 8 */
	unsigned short BitsPerSample;   /**< 每个采样点的位数，8 位 / 16 位 / 32 位 */

	char Subchunk2ID[4];            /**< 固定为 "data" */
	unsigned int Subchunk2Size;     /**< 数据大小，NumChannels * SampleRate * BitsPerSample / 8 */
} WAVE_HEADER;

#pragma pack()

class ScWaveFormat : public ScSignalFormat
{
public:
	ScWaveFormat();
	~ScWaveFormat() = default;

	bool isValid(const char* inBuff, int inlen) override;
	int headerLength() const override { return sizeof(WAVE_HEADER); }
	double ratio() override;
};

#endif // SCWAVEFORMAT_H
