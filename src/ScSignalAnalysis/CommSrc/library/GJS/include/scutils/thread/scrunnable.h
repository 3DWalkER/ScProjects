#ifndef SCRUNNABLE_H
#define SCRUNNABLE_H

#include "scutils/scglobal.h"
#include <functional>

class SC_API_EXPORT ScRunnable
{
public:
    virtual ~ScRunnable() = default;

    /**
     * @brief run 线程任务
     */
    virtual void run() = 0;

    /**
     * @brief isAutoDelete 是否自动释放
     */
    bool isAutoDelete() const { return m_isAutoDelete; }

    /**
     * @brief setAutoDelete 设置是否自动释放，如果是临时变量，必须设置为false
     */
    void setAutoDelete(bool isAuto) { m_isAutoDelete = isAuto; }

    /**
     * @brief create 
     */
    static ScRunnable* create(std::function<void()> funcToRun);

private:
    /**
     * @brief m_isAutoDelete 任务执行完成后是否主动自动释放
     */
    bool m_isAutoDelete{ true };
};

#endif // SCRUNNABLE_H
