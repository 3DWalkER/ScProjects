#ifndef SCDETACHABLETASK_H
#define SCDETACHABLETASK_H

#include "scatomic.h"
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class ScDetachableTask : public std::enable_shared_from_this<ScDetachableTask<T>>
{
public:
	/**
	 * @brief start 启动线程任务
	 */
	void start();

	/**
	 * @brief stop 停止线程任务，主动退出时放弃对线程的控制，使其在后台自行结束
	 */
	void stop();

	/**
	 * @brief wait 等待线程任务结束
	 * @param msecs 等待毫秒数，-1表示永久等待
	 */
	void wait(int msecs = -1);

	/**
	 * @brief value 获取运行结果
	 */
	std::shared_ptr<T> value() const {
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_value;
	}

private:
	enum class State
	{
		Idle,       /**< 闲置 */
		Running,    /**< 运行中 */
		Detached    /**< 已脱离 */
	};

	ScAtomicInteger<State> m_state{ State::Idle };  /**< 线程任务状态 */
	std::thread m_taskTh;			/**< 线程任务线程 */
	std::shared_ptr<T> m_value;		/**< 线程任务结果 */
	mutable std::mutex m_mutex;		/**< 线程任务结果访问互斥锁 */
	std::condition_variable m_waitCondi; /** < 线程退出的条件变量 */

private:
	/**
	 * @brief executed 任务线程执行内容
	 */
	void executed();

protected:
	virtual T run() = 0; /**< 任务线程执行的具体内容，由子类实现 */
};

template<typename T>
inline void ScDetachableTask<T>::start()
{
	if (!m_state.testAndSetOrdered(State::Idle, State::Running))
		return;

	try {
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_value.reset();
		}

		auto self = this->shared_from_this();
		m_taskTh = std::thread([this, self]() { this->executed(); });
	}
	catch (...) {
		m_state.storeRelease(State::Idle);
		throw;
	}
}

template<typename T>
inline void ScDetachableTask<T>::stop()
{
	if (!m_state.testAndSetOrdered(State::Running, State::Detached))
		return;

	if (m_taskTh.joinable())
		m_taskTh.detach();

	{
		std::lock_guard<std::mutex> locker(m_mutex);
	}
	m_waitCondi.notify_all();
}

template<typename T>
inline void ScDetachableTask<T>::wait(int msecs)
{
	std::unique_lock<std::mutex> locker(m_mutex);
	auto predicate = [this] { return m_state.loadAcquire() != State::Running; };
	if (msecs >= 0)
	{
		if (!m_waitCondi.wait_for(locker, std::chrono::milliseconds(msecs), predicate))
		{
			locker.unlock();
			stop();
			return;
		}
	}
	else
		m_waitCondi.wait(locker, predicate);

	if (State::Idle == m_state.loadAcquire())
	{
		if (m_taskTh.joinable())
			m_taskTh.join();
	}
}

template<typename T>
inline void ScDetachableTask<T>::executed()
{
	try {
		T ret = std::move(run());
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_state.loadAcquire() == State::Running)
				m_value = std::make_shared<T>(std::move(ret));
		}
	}
	catch (...) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_value.reset();
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_state.testAndSetOrdered(State::Running, State::Idle);
		m_waitCondi.notify_all();
	}
}

#endif // SCDETACHABLETASK_H
