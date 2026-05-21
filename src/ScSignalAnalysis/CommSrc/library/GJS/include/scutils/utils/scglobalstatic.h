#ifndef SCGLOBAL_H
#   include "scutils/scglobal.h"
#endif

#ifndef SCGLOBALSTATIC_H
#define SCGLOBALSTATIC_H

#include "scutils/thread/scatomic.h"

enum ScGuardValues
{
	Destroyed = -2,
	Initialized = -1,
	Uninitialized = 0,
	Initializing = 1
};

#if defined(SC_COMPILER_THREADSAFE_STATICS)
#	if defined(SC_OS_UNIX) && defined(SC_CC_INTEL)
#		define SC_GLOBAL_STATIC_INTERNAL_DECORATION SC_DECL_HIDDEN
#	else
#		define SC_GLOBAL_STATIC_INTERNAL_DECORATION SC_DECL_HIDDEN inline
#	endif
#   define SC_GLOBAL_STATIC_INTERNAL(ARGS)                                  \
        SC_GLOBAL_STATIC_INTERNAL_DECORATION Type *innerFunction()          \
        {                                                                   \
            struct HolderBase {                                             \
                ~HolderBase() noexcept {                                    \
                    if (guard.loadRelaxed() == ScGuardValues::Initialized)  \
                        guard.storeRelaxed(ScGuardValues::Destroyed);       \
                }                                                           \
            };                                                              \
            static struct Holder : public HolderBase {                      \
                Type value;                                                 \
                Holder() noexcept(noexcept(Type ARGS))                      \
                    : value ARGS                                            \
                { guard.storeRelaxed(ScGuardValues::Initialized); }         \
            } holder;                                                       \
            return &holder.value;                                           \
        }
#else
#	define SC_GLOBAL_STATIC_INTERNAL(ARGS)									\
		SC_DECL_HIDDEN inline Type *innerFunction()							\
		{																	\
			static Type *d;													\
			static std::mutex mutex;										\
			int x = guard.loadAcquire();									\
			if (x >= ScGuardValues::Uninitialized)							\
			{																\
				const std::lock_guard<std::mutex> locker(mutex);			\
				if (guard.loadRelaxed() == ScGuardValues::Uninitialized)	\
				{															\
					d = new Type ARGS;										\
					static struct Cleanup {									\
						~Cleanup() {										\
							delete d;										\
							guard.storeRelaxed(ScGuardValues::Destroyed);	\
						}													\
					} cleanup;												\
					guard.storeRelease(ScGuardValues::Initialized);			\
				}															\
			}																\
			return d;														\
		}
#endif

template <typename T, T* (&innerFunction)(), ScAtomicInt& guard>
struct ScGlobalStatic
{
    typedef T Type;

    bool isDestroyed() const { return guard.loadRelaxed() <= ScGuardValues::Destroyed; }
    bool exists() const { return guard.loadRelaxed() == ScGuardValues::Initialized; }
    operator Type* () { if (isDestroyed()) return nullptr; return innerFunction(); }
    Type* operator()() { if (isDestroyed()) return nullptr; return innerFunction(); }
    Type* operator->()
    {
        SC_ASSERT_X(!isDestroyed(), "SC_GLOBAL_STATIC", "The global static was used after being destroyed");
        return innerFunction();
    }
    Type& operator*()
    {
        SC_ASSERT_X(!isDestroyed(), "SC_GLOBAL_STATIC", "The global static was used after being destroyed");
        return *innerFunction();
    }
};

#define SC_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)	\
	namespace { namespace SCGS_ ## NAME {				\
		typedef TYPE Type;								\
		ScAtomicInt guard(ScGuardValues::Uninitialized);		\
		SC_GLOBAL_STATIC_INTERNAL(ARGS)					\
	} }													\
	static ScGlobalStatic<TYPE, SCGS_ ## NAME::innerFunction, SCGS_ ## NAME::guard> NAME;

#define SC_GLOBAL_STATIC(TYPE, NAME) \
	SC_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ())

#endif // SCGLOBALSTATIC_H
