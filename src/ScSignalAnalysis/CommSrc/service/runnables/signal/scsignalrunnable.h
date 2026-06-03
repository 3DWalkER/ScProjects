#ifndef SCSIGNALMANAGER_H
#define SCSIGNALMANAGER_H

#include "scrunnablemanager.h"
#include "scanalysisservice.h"
#include "scserviceglobal.h"

class ScSignalRunnable : public ScDefinedRunnable<ScAnalysisService, ScRunnableManager>
{
public:
	ScSignalRunnable(Sc::SignalTaskType taskType, const usrid &id, int channel = 0, int subChannel = 0);
	~ScSignalRunnable() override;
};

#endif // SCSIGNALMANAGER_H