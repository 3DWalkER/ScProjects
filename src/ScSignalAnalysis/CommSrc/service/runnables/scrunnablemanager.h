#ifndef SCRUNNABLEMANAGER_H
#define SCRUNNABLEMANAGER_H

#include "scutils/thread/scrunnable.h"
#include "scutils/thread/screadwritelock.h"
#include "scabstractservice.h"
#include "scobjecpool.h"

#include <mutex>
#include <unordered_map>

class ScRunnableManager;

class ScSaRunnable : public ScRunnable, public std::enable_shared_from_this<ScSaRunnable>
{
	friend class ScRunnableManager;
public:
	explicit ScSaRunnable(int taskType, const usrid& id, int channel = 0, int subChannel = 0)
		: taskType(taskType), id(id), channel(channel), subChannel(subChannel) {
		setAutoDelete(false);
	}
	~ScSaRunnable() override = default;

	/**
	 * @brief isRunning 是否正在运行
	 */
	bool isRunning() const { return m_isRunning.loadRelaxed(); }
	void setEnabled(bool on) { return m_isRunning.storeRelease(on); }

	/**
	 * @brief stop 停止线程任务
	 */
	virtual void stop() { setEnabled(false); }

	bool isAborted() const { return m_isAbort.loadAcquire(); }
	void abort() { m_isAbort.storeRelease(true); stop(); }

private:
	bool waitForDone(int msecs = -1);

	ScAtomicInteger<bool> m_isAbort{ false };	/**< 是否已经终止 */
	ScAtomicInteger<bool> m_isRunning{ false };	/**< 线程任务状态 */

protected:
	/**
	 * @brief sendStatus 发送任务状态
	 * @param type          [in]状态类型
	 * @param message       [in]状态信息
	 */
	void sendStatus(Sc::Status type, const ScString& message);

	int taskType{ };			/**< 线程任务 */
	usrid id;					/**< 用户唯一ID */
	int channel;				/**< 任务通道号 */
	int subChannel;				/**< 任务子通道号 */

	ScRunnableManager* m_pRunnableMgr{ }; /**< 任务管理器 */

	bool isFinished{ false };		/**< 任务是否已经完成 */
	std::mutex waitMutex;			/**< 等待结束的互斥锁 */
	std::condition_variable waitCv;	/**< 等待结束的条件变量 */

protected:
	/**
	 * @brief test 测试该线程任务能否托管给指定线程任务管理器
	 */
	virtual bool test(ScRunnableManager* mgr) const = 0;

	/**
	 * @brief test 测试该线程任务能否托管给指定的服务
	 */
	virtual bool test(ScAbstractService* service) const = 0;

	/**
	 * @brief executed 派生类重写此函数实现具体业务
	 */
	virtual int executed() = 0;

	/**
	 * @brief run 重写函数
	 */
	void run() override;
};
using ScRunnablePtr = std::shared_ptr<ScSaRunnable>;


/**
 * @brief class ScRunnableManager 线程任务管理器
 */
class ScRunnableManager
{
	friend class ScSaRunnable;
public:
	ScRunnableManager();
	~ScRunnableManager();

	/**
	 * @brief start 启动线程任务
	 * @param runnable			[in]线程任务
	 * @return true：启动成功，false：启动失败
	 */
	bool start(ScRunnablePtr runnable);

	/**
	 * @brief stop 停止并移除指定任务
	 * @return true：找到并停止了任务；false：未找到该任务
	 */
	void stop(const usrid& userId, int channel, int subChannel);

	/**
	 * @brief stopAndWait 停止并等待指定任务结束
	 * @return true：找到并停止了任务；false：未找到该任务
	 */
	bool stopAndWait(const usrid& userId, int channel, int subChannel, int millsec);

	/**
	 * @brief isExistsActive 判断指定的活跃任务是否存在
	 * @param runnable			[in]线程任务
	 * @return true：存在，false：不存在
	 */
	bool isExistsActive(const ScRunnablePtr& runnable);
	bool isExistsActive(const usrid& userId, int channel, int subChannel);

	/**
	 * @brief removeActive 移除处于活跃状态的线程任务
	 */
	void removeActive(const ScRunnablePtr& runnable);
	void removeActive(const usrid& userId, int channel, int subChannel);

	/**
	 * @brief maxThreadCount 线程池使用的最大线程数，默认为逻辑处理器数
	 */
	int maxThreadCount() const;
	void setMaxThreadCount(int maxCount);

	/**
	 * @brief service 该线程任务管理器所属服务
	 */
	ScAbstractService* service() const { return m_pService; }

private:
	/**
	 * @brief The State enum 管理器的状态
	 */
	enum class State
	{
		Opened,		/**< 已开启 */
		Closing,	/**< 关闭中 */
		Closed		/**< 已关闭 */
	};

	ScBasicAtomicInteger<State> state;	/**< 任务管理器状态 */
	ScAbstractService* m_pService{ };	/**< 任务归属服务 */
	class ScThreadPool* m_pThreadPool;	/**< 线程池 */

	using ScRunnables = ScExternalPool<ScRunnablePtr>;
	ScRunnables m_activeRunnables;		/**< 活跃中的线程任务 */

	/**
	 * @brief close 关闭该线程任务管理器
	 */
	void close();

protected:
	/**
	 * @brief sendStatus 发送任务状态
	 * @param runnable			[in]发送状态的任务体
	 * @param type				[in]状态类型
	 * @param message			[in]状态信息
	 */
	void sendStatus(ScRunnablePtr runnable, Sc::Status type, const ScString& message) {
		if (m_pService && runnable)
			m_pService->sendStatus(runnable->id, runnable->channel, runnable->subChannel, runnable->taskType, type, message);
	}
};

inline bool ScRunnableManager::isExistsActive(const ScRunnablePtr& runnable)
{
	return runnable && isExistsActive(runnable->id, runnable->channel, runnable->subChannel);
}

inline bool ScRunnableManager::isExistsActive(const usrid& userId, int channel, int subChannel)
{
	return nullptr != m_activeRunnables.tryAcquire(userId, ScChannelKey{ channel, subChannel });
}

inline void ScRunnableManager::removeActive(const ScRunnablePtr& runnable)
{
	if (runnable)
		removeActive(runnable->id, runnable->channel, runnable->subChannel);
}

inline void ScRunnableManager::removeActive(const usrid& userId, int channel, int subChannel)
{
	m_activeRunnables.release(userId, ScChannelKey{ channel, subChannel });
}

inline void ScSaRunnable::sendStatus(Sc::Status type, const ScString& message)
{
	m_pRunnableMgr->sendStatus(shared_from_this(), type, message);
}


/**
 * @brief The ScDefinedRunnable class 线程任务实现
 */
template <class Service, class RunnableManager>
class ScDefinedRunnable : public ScSaRunnable
{
public:
	explicit ScDefinedRunnable(int taskType, const usrid& id, int channel = 0, int subChannel = 0)
		: ScSaRunnable(taskType, id, channel, subChannel) {
	}
	~ScDefinedRunnable() override = default;

protected:
	bool test(ScRunnableManager* mgr) const override { return nullptr != dynamic_cast<RunnableManager*>(mgr); }
	bool test(ScAbstractService* service) const override { return nullptr != dynamic_cast<Service*>(service); }

	inline RunnableManager* runnableManager() const { return static_cast<RunnableManager*>(m_pRunnableMgr); }
	inline Service* service() const { return static_cast<Service*>(m_pRunnableMgr->service()); }
};

#endif // SCRUNNABLEMANAGER_H