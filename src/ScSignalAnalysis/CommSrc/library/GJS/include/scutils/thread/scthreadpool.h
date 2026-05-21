#ifndef SCTHREADPOOL_H
#define SCTHREADPOOL_H

#include "scutils/scglobal.h"

#include <functional>

class ScRunnable;
class ScThreadPoolPrivate;

class SC_API_EXPORT ScThreadPool
{
	SC_DECLARE_PRIVATE(ScThreadPool)
public:
	ScThreadPool();
	~ScThreadPool();

	void start(ScRunnable* task, int priority = 0);
	void start(std::function<void()> funcToRun, int priority = 0);
	bool tryStart(ScRunnable* task);
	bool tryStart(std::function<void()> funcToRun);

	int expiryTimeout() const;
	void setExpiryTimeout(int msecs);

	int maxThreadCount() const;
	void setMaxThreadCount(int maxCount);

	int activeThreadCount() const;

	scuint32 stackSize() const;
	void setStackSize(scuint32 stackSize);

	void reserveThread();
	void releaseThread();

	bool waitForDone(int msecs = -1);

	void clear();

private:
	ScThreadPoolPrivate *d_ptr;
};

#endif // SCTHREADPOOL_H
