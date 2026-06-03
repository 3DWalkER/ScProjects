#include "scplaybackrunnable.h"
#include "scsignalrunnable.h"

ScSignalRunnable::ScSignalRunnable(Sc::SignalTaskType taskType, const usrid& id, int channel, int subChannel)
	: ScDefinedRunnable<ScAnalysisService, ScRunnableManager>(taskType, id, channel, subChannel)
{
}

ScSignalRunnable::~ScSignalRunnable()
{
}