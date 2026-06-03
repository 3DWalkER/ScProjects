#ifndef SCANALYSISSERVER_H
#define SCANALYSISSERVER_H

#include "scabstractservice.h"

/**
 * @brief The CDAnalysisService class 用于信号分析的抽象服务
 */
class ScAnalysisService : public ScAbstractService
{
public:
	virtual ScSaWorkParam basicWorkParam(const usrid& id, int channel, int subChannel) const = 0;

	ScMetaData metaData(const usrid& id, int channel, int subChannel) const { basicWorkParam(id, channel, subChannel).meta; }
};

#endif // SCANALYSISSERVER_H
