#ifndef SCPLAYBACKRUNNABLE_H
#define SCPLAYBACKRUNNABLE_H

#include "scsignalrunnable.h"

class ScPlaybackRunnable : public ScSignalRunnable
{
public:
	explicit ScPlaybackRunnable(const usrid& id, int channel = 0, int subChannel = 0);
	~ScPlaybackRunnable() override;

protected:
	int executed() override;
};

#endif // SCPLAYBACKRUNNABLE_H