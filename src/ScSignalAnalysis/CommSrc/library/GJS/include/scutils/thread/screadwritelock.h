#ifndef SCREADWRITELOCK_H
#define SCREADWRITELOCK_H

#include "scutils/scglobal.h"
#include "scutils/thread/scatomic.h"

SC_WARNING_PUSH
SC_WARNING_DISABLE_MSVC(4251)

class ScReadWriteLockPrivate;

class SC_API_EXPORT ScReadWriteLock
{
	SC_DISABLE_COPY(ScReadWriteLock)
public:
	enum RecursionMode
	{
		NonRecursive,
		Recursive
	};

	explicit ScReadWriteLock(RecursionMode recursionMode = NonRecursive);
	~ScReadWriteLock();

	void lockForRead();
	bool tryLockForRead(int timeout = 0);

	void lockForWrite();
	bool tryLockForWrite(int timeout = 0);

	void unlock();

private:
	ScAtomicPointer<ScReadWriteLockPrivate> d_ptr{ nullptr };
};

class ScReadLocker
{
public:
	inline ScReadLocker(ScReadWriteLock* readWriteLock);

	inline ~ScReadLocker()
	{
		unlock();
	}

	inline void unlock()
	{
		if (q_val)
		{
			if ((q_val & scuintptr(1u)) == scuintptr(1u))
			{
				q_val &= ~scuintptr(1u);
				readWriteLock()->unlock();
			}
		}
	}

	inline void relock()
	{
		if (q_val)
		{
			if ((q_val & scuintptr(1u)) == scuintptr(0u))
			{
				readWriteLock()->lockForRead();
				q_val |= scuintptr(1u);
			}
		}
	}

	inline ScReadWriteLock* readWriteLock() const
	{
		return reinterpret_cast<ScReadWriteLock*>(q_val & ~scuintptr(1u));
	}

private:
	SC_DISABLE_COPY(ScReadLocker)
	scuintptr q_val;
};

inline ScReadLocker::ScReadLocker(ScReadWriteLock* areadWriteLock)
	: q_val(reinterpret_cast<scuintptr>(areadWriteLock))
{
	SC_ASSERT_X((q_val & scuintptr(1u)) == scuintptr(0), "ScReadLocker", "ScReadWriteLock pointer is misaligned");
	relock();
}

class ScWriteLocker
{
public:
	inline ScWriteLocker(ScReadWriteLock* readWriteLock);

	inline ~ScWriteLocker()
	{
		unlock();
	}

	inline void unlock()
	{
		if (q_val)
		{
			if ((q_val & scuintptr(1u)) == scuintptr(1u))
			{
				q_val &= ~scuintptr(1u);
				readWriteLock()->unlock();
			}
		}
	}

	inline void relock()
	{
		if (q_val)
		{
			if ((q_val & scuintptr(1u)) == scuintptr(0u))
			{
				readWriteLock()->lockForWrite();
				q_val |= scuintptr(1u);
			}
		}
	}

	inline ScReadWriteLock* readWriteLock() const
	{
		return reinterpret_cast<ScReadWriteLock*>(q_val & ~scuintptr(1u));
	}

private:
	SC_DISABLE_COPY(ScWriteLocker)
	scuintptr q_val;
};

inline ScWriteLocker::ScWriteLocker(ScReadWriteLock* areadWriteLock)
	: q_val(reinterpret_cast<scuintptr>(areadWriteLock))
{
	SC_ASSERT_X((q_val & scuintptr(1u)) == scuintptr(0), "ScWriteLocker", "ScReadWriteLock pointer is misaligned");
	relock();
}

SC_WARNING_POP

#endif // SCREADWRITELOCK_H
