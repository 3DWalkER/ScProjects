#ifndef SCABSTRACTSERVER_H
#define SCABSTRACTSERVER_H

#include "scsaglobal.h"

/**
 * @brief The ScAbstractService class 抽象服务
 */
class ScAbstractService
{
public:
    /**
     * @brief open 打开分析服务
     * @return true：成功，false：失败
     */
    virtual bool open() = 0;

    /**
     * @brief sendStatus 发送工作状态
     * @param id                [in]用户唯一标识ID
     * @param channel           [in]通道号
     * @param subChannel        [in]子通道
     * @param taskType          [in]任务类型
     * @param type              [in]状态类型
     * @param msg               [in]状态报文信息
     */
    virtual void sendStatus(const usrid& id, int channel, int subChannel, int taskType, Sc::Status type, const ScString& msg) const = 0;

    /**
     * @brief requestData 请求数据
     * @param id                [in]用户唯一标识ID
     * @param channel           [in]通道号
     * @param subChannel        [in]子通道
     * @param begin             [in]数据起始位置
     * @param end               [in]数据结束位置
     */
    virtual void requestData(const usrid& id, int channel, int subChannel, scuint64 begin, scuint64 end) = 0;

    /**
     * @brief close 关闭服务
     */
    virtual void close() = 0;
};

#endif // SCABSTRACTSERVER_H
