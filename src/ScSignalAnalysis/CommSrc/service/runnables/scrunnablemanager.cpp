#include "scrunnablemanager.h"

#include "scutils/thread/scthreadpool.h"

void ScSaRunnable::run()
{
	ScRunnablePtr self = shared_from_this();
	if (!m_pRunnableMgr)
	{
		SA_DEBUG("任务处于游离状态，未绑定任务管理器！");
		return;
	}

	if (isAborted())
	{
		m_pRunnableMgr->removeActive(self);
		return;
	}

	setEnabled(true);

	int ret = Sc::SUCCESS;
	try {
		ret = executed();
	}
	catch (...) {
		sendStatus(Sc::S_Critical, "任务运行期间发生异常中断！");
	}

	setEnabled(false);

	{
		std::lock_guard<std::mutex> locker(waitMutex);
		isFinished = true;
	}
	waitCv.notify_all();
	m_pRunnableMgr->removeActive(self);
}

bool ScSaRunnable::waitForDone(int msecs)
{
	std::unique_lock<std::mutex> locker(waitMutex);
	if (msecs < 0)
	{
		waitCv.wait(locker, [this]() { return isFinished; });
		return true;
	}
	else
		return waitCv.wait_for(locker, std::chrono::milliseconds(msecs), [this]() { return isFinished; });
}

ScRunnableManager::ScRunnableManager()
	: state(State::Opened)
	, m_pThreadPool(new ScThreadPool())
{
}

ScRunnableManager::~ScRunnableManager()
{
	close();
}

bool ScRunnableManager::start(ScRunnablePtr runnable)
{
	if (!runnable)
		return false;

	if (State::Closing == state.loadAcquire())
	{
		sendStatus(runnable, Sc::S_Warn, "任务启动失败：任务管理器正在关闭，无法启动新任务！");
		return false;
	}

	if (!runnable->test(m_pService))
	{
		sendStatus(runnable, Sc::S_Debug, "任务启动失败：服务配置不匹配，无法启动任务！");
		return false;
	}

	if (!runnable->test(this))
	{
		sendStatus(runnable, Sc::S_Debug, "任务启动失败：任务管理器配置不匹配，无法启动任务！");
		return false;
	}

	runnable->m_pRunnableMgr = this;
	if (!m_activeRunnables.emplace(runnable->id, ScChannelKey{ runnable->channel, runnable->subChannel }, runnable))
	{
		sendStatus(runnable, Sc::S_Warn, "任务启动失败：该任务已执行，无法再次启动！");
		return false;
	}

	m_pThreadPool->start(runnable.get());
	return true;
}

void ScRunnableManager::stop(const usrid& userId, int channel, int subChannel)
{
	ScRunnablePtr pTarget = m_activeRunnables.tryAcquire(userId, channel);
	if (pTarget)
		pTarget->abort();
}

bool ScRunnableManager::stopAndWait(const usrid& userId, int channel, int subChannel, int millsec)
{
	ScRunnablePtr pTarget = m_activeRunnables.tryAcquire(userId, channel);
	if (!pTarget)
		return true;

	pTarget->abort();
	return pTarget->waitForDone(millsec);
}

int ScRunnableManager::maxThreadCount() const
{
	return m_pThreadPool->maxThreadCount();
}

void ScRunnableManager::setMaxThreadCount(int maxCount)
{
	maxCount = maxCount <= 0 ? std::thread::hardware_concurrency() : maxCount;
	m_pThreadPool->setMaxThreadCount(maxCount);
}

void ScRunnableManager::close()
{
	if (!state.testAndSetOrdered(State::Opened, State::Closing))
		return;

	m_pThreadPool->waitForDone();
	SC_SAVE_DELETE(m_pThreadPool);
	state.storeRelease(State::Closed);
}
