#include "scplaybackrunnable.h"

ScPlaybackRunnable::ScPlaybackRunnable(const usrid& id, int channel, int subChannel)
	: ScSignalRunnable(Sc::STT_Playback, id, channel, subChannel)
{

}

ScPlaybackRunnable::~ScPlaybackRunnable()
{

}

int ScPlaybackRunnable::executed()
{
	int ret = Sc::SUCCESS;
	while (isRunning())
	{
		_sleep(10);
	}
	return ret;
}
