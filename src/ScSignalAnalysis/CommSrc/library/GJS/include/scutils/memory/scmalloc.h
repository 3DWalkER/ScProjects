#ifndef SCMALLOC_H
#define SCMALLOC_H

#include "scutils/scglobal.h"
#include "scutils/memory/scmallocimpl.h"

#include <new>
#include <atomic>

#if defined(SC_CC_GNU)
#	define SC_MALLOC_CHECKED_MALLOC __attribute__((__returns_nonnull__, __malloc__))
#else
#	define SC_MALLOC_CHECKED_MALLOC
#endif

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

#if SC_CPLUSPLUS >= 202002L
template <class Initializer>
class ScFastStaticBool
{
public:
	SC_ALWAYS_INLINE static bool get(std::memory_order mo = std::memory_order_acquire) noexcept {
		auto f = flag_.load(mo);
		if (SC_LIKELY(f != 0))
			return f > 0;
		return getSlow();
	}

private:
	SC_ATTR_GUN_COLD SC_NEVER_INLINE SC_EXPORT static bool getSlow() noexcept
	{
		static bool rv = [] {
			auto v = Initializer{}();
			flag_.store(v ? 1 : -1, std::memory_order_release);
			return v;
		}();
		return rv;
	}

	static std::atomic<signed char> flag_;
};

template <class Initializer>
constinit std::atomic<signed char> ScFastStaticBool<Initializer>::flag_{};
#else // SC_CPLUSPLUS >= 202002L
template <class Initializer>
class ScFastStaticBool
{
public:
	SC_ALWAYS_INLINE static bool get(std::memory_order = std::memory_order_acquire) noexcept {
		static const bool rv = Initializer{}();
		return rv;
	}
};
#endif

struct ScUsingJEMallocInitializer {
	bool operator()() const noexcept;
};

struct ScUsingTCMallocInitializer {
	bool operator()() const noexcept;
};

SC_END_DETAIL_NAMESPACE

#if defined(SC_ASSUME_NO_JEMALLOC) || defined(SC_SANITIZE)
#	define SC_CONSTANT_USING_JE_MALLOC 1
inline bool isUsingJEMalloc() noexcept {
	return false;
}
#elif defined(USE_JEMALLOC) && !defined(SC_SANITIZE)
#	define SC_CONSTANT_USING_JE_MALLOC 1
inline bool isUsingJEMalloc() noexcept {
	return true;
}
#else
#	define SC_CONSTANT_USING_JE_MALLOC 0
inline bool isUsingJEMalloc() noexcept {
	using Initializer = detail::ScUsingJEMallocInitializer;
	return detail::ScFastStaticBool<Initializer>::get(std::memory_order_relaxed);
}
#endif

#if defined(SC_ASSUME_NO_TCMALLOC) || defined(SC_SANITIZE)
#	define SC_CONSTANT_USING_TC_MALLOC 1
inline bool isUsingTCMalloc() noexcept {
	return false;
}
#elif defined(USE_TCMALLOC) && !defined(SC_SANITIZE)
#	define SC_CONSTANT_USING_TC_MALLOC 1
inline bool isUsingTCMalloc() noexcept {
	return true;
}
#else
#	define SC_CONSTANT_USING_TC_MALLOC 0
inline bool isUsingTCMalloc() noexcept {
	using Initializer = detail::ScUsingTCMallocInitializer;
	return detail::ScFastStaticBool<Initializer>::get(std::memory_order_relaxed);
}
#endif

inline bool isUsingJEMallocOrTCMalloc() noexcept {
	return isUsingJEMalloc() || isUsingTCMalloc();
}

inline bool isSdallocxEnabled() noexcept {
	return isUsingJEMallocOrTCMalloc();
}

inline bool isNallocxEnabled() noexcept {
	return isUsingJEMallocOrTCMalloc();
}

inline size_t goodMallocSize(size_t minSize) noexcept
{
	if (0 == minSize)
		return 0;

	if (!isNallocxEnabled())
		return minSize;

	auto rv = nallocx(minSize, 0);
	return rv ? rv : minSize;
}

inline void* checkedMalloc(size_t size) {
	void* p = malloc(size);
	if (!p)
		throw std::bad_alloc();
	return p;
}

inline void* checkedCalloc(size_t n, size_t size) {
	void* p = calloc(n, size);
	if (!p)
		throw std::bad_alloc();
	return p;
}

inline void* checkedRealloc(void* ptr, size_t size) {
	void* p = realloc(ptr, size);
	if (!p)
		throw std::bad_alloc();
	return p;
}

SC_MALLOC_CHECKED_MALLOC SC_NEVER_INLINE inline void* smartRealloc(void *p, const size_t currSize, const size_t currCapacity, const size_t newCapacity)
{
	SC_ASSERT(p);
	SC_ASSERT(currSize <= currCapacity && currCapacity < newCapacity);
	auto const slack = currCapacity - currSize;
	if (slack * 2 > currSize)
	{
		auto const result = checkedMalloc(newCapacity);
		memcpy(result, p, currSize);
		free(p);
		return result;
	}
	return checkedRealloc(p, newCapacity);
}

SC_END_NAMESPACE

#endif // SCMALLOC_H