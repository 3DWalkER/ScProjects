#ifndef SCVSIFORMAT_H
#define SCVSIFORMAT_H

#include "scsignalformat.h"

#pragma pack(1)

struct VSI_HEADER
{
	unsigned int	record_label;	/** ¼ÇÂ¼±êÇ© **/
};

#pragma pack()

class ScVsiFormat : public ScSignalFormat
{
public:
	ScVsiFormat();
	~ScVsiFormat() = default;

	bool isValid(const char* inBuff, int inlen) override;
	int headerLength() const override { return sizeof(VSI_HEADER); }
	double ratio() override { return bitDepth() <= 0 ? 1.0 : 8.0 / bitDepth(); }
};

#endif // SCVSIFORMAT_H

