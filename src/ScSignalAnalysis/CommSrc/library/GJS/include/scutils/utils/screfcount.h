#ifndef SCREFCOUNT_H
#define SCREFCOUNT_H

#include "scutils/scglobal.h"
#include "scutils/thread/scatomic.h"

class ScRefCount
{
public:
	inline bool ref() noexcept
	{
		int count = atomic.loadRelaxed();
		if (0 == count)
			return false;

		if (-1 != count)
			atomic.ref();
		return true;
	}

	inline bool deref() noexcept
	{
		int count = atomic.loadRelaxed();
		if (0 == count)
			return false;

		if (-1 == count)
			return true;
		return atomic.deref();
	}

	bool setSharable(bool sharable) noexcept
	{
		SC_ASSERT(!isShared());
		if (sharable)
			return atomic.testAndSetRelaxed(0, 1);
		else
			return atomic.testAndSetRelaxed(1, 0);
	}

	bool isSharable() const noexcept
	{
		return atomic.loadRelaxed() != 0;
	}

	bool isStatic() const noexcept
{
		return atomic.loadRelaxed() == -1;
	}

	bool isShared() const noexcept
	{
		int count = atomic.loadRelaxed();
		return (count != 1) && (count != 0);
	}

	void initializeOwned() noexcept { atomic.storeRelaxed(1); }
	void initializeUnsharable() noexcept { atomic.storeRelaxed(0); }

	ScBasicAtomicInt atomic;
};

#define SC_REFCOUNT_INITIALIZE_STATIC { ATOMIC_VAR_INIT(-1) }

#endif // SCREFCOUNT_H
