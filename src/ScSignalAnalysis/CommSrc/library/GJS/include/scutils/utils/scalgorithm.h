#ifndef SCALGORITHM_H
#define SCALGORITHM_H

#include "scutils/scglobal.h"
#ifdef SC_CC_MSVC
#   include <intrin.h>
#endif

SC_BEGIN_NAMESPACE

#ifdef SC_CC_CLANG
// Clang had a bug where __builtin_ctz/clz/popcount were not marked as constexpr.
#	if (defined __apple_build_version__ &&  __clang_major__ >= 7) || (SC_CC_CLANG >= 307)
#		define SC_HAS_CONSTEXPR_BUILTINS
#	endif
#elif defined(SC_CC_MSVC) && !defined(SC_CC_INTEL) && !defined(SC_PROCESSOR_ARM)
#	define SC_HAS_CONSTEXPR_BUILTINS
#elif defined(SC_CC_GNU)
#	define SC_HAS_CONSTEXPR_BUILTINS
#endif

#ifdef SC_HAS_CONSTEXPR_BUILTINS
#if defined(SC_CC_GNU) || defined(SC_CC_CLANG)
#define SC_HAS_BUILTIN_CTZS
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scint32 builtin_ctzs(scint16 v) noexcept
{
#  if __has_builtin(__builtin_ctzs)
    return __builtin_ctzs(v);
#  else
    return __builtin_ctz(v);
#  endif
}

#define SC_HAS_BUILTIN_CLZS
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_clzs(scuint16 v) noexcept
{
#  if __has_builtin(__builtin_clzs)
    return __builtin_clzs(v);
#  else
    return __builtin_clz(v) - 16U;
#  endif
}

#define SC_HAS_BUILTIN_CTZ
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_ctz(scuint32 v) noexcept {
    return __builtin_ctz(v);
}

#define SC_HAS_BUILTIN_CLZ
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_clz(scuint32 v) noexcept {
    return __builtin_clz(v);
}

#define SC_HAS_BUILTIN_CTZLL
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_ctzll(scuint64 v) noexcept {
    return __builtin_ctzll(v);
}

#define SC_HAS_BUILTIN_CLZLL
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_clzll(scuint64 v) noexcept {
    return __builtin_clzll(v);
}

#define SC_ALGORITHMS_USE_BUILTIN_POPCOUNT
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_popcount(scuint32 v) noexcept {
    return __builtin_popcount(v);
}

SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_popcount(quint8 v) noexcept {
    return __builtin_popcount(v);
}

SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_popcount(scuint16 v) noexcept {
    return __builtin_popcount(v);
}

#define SC_ALGORITHMS_USE_BUILTIN_POPCOUNTLL
SC_DECL_CONSTEXPR SC_ALWAYS_INLINE scuint32 builtin_popcountll(scuint64 v) noexcept {
    return __builtin_popcountll(v);
}
#elif defined(SC_CC_MSVC) && !defined(SC_PROCESSOR_ARM)
#define SC_POPCOUNT_CONSTEXPR
#define SC_POPCOUNT_RELAXED_CONSTEXPR
#define SC_HAS_BUILTIN_CTZ
SC_ALWAYS_INLINE unsigned long builtin_ctz(scuint32 val)
{
    unsigned long result;
    _BitScanForward(&result, val);
    return result;
}

#define SC_HAS_BUILTIN_CLZ
SC_ALWAYS_INLINE unsigned long builtin_clz(scuint32 val)
{
    unsigned long result;
    _BitScanReverse(&result, val);
    result ^= sizeof(scuint32) * 8 - 1;
    return result;
}

#if SC_PROCESSOR_WORDSIZE == 8
// These are only defined for 64bit builds.
#define SC_HAS_BUILTIN_CTZLL
SC_ALWAYS_INLINE unsigned long builtin_ctzll(scuint64 val)
{
    unsigned long result;
    _BitScanForward64(&result, val);
    return result;
}

// MSVC calls it _BitScanReverse and returns the carry flag, which we don't need
#define SC_HAS_BUILTIN_CLZLL
SC_ALWAYS_INLINE unsigned long builtin_clzll(scuint64 val)
{
    unsigned long result;
    _BitScanReverse64(&result, val);
    // see builtin_clz
    result ^= sizeof(scuint64) * 8 - 1;
    return result;
}
#endif // MSVC 64bit
#  define SC_HAS_BUILTIN_CTZS
SC_ALWAYS_INLINE scuint32 builtin_ctzs(scuint16 v) noexcept {
    return builtin_ctz(v);
}

#define SC_HAS_BUILTIN_CLZS
SC_ALWAYS_INLINE scuint32 builtin_clzs(scuint16 v) noexcept {
    return builtin_clz(v) - 16U;
}

// Neither MSVC nor the Intel compiler define a macro for the POPCNT processor
// feature, so we're using either the SSE4.2 or the AVX macro as a proxy (Clang
// does define the macro). It's incorrect for two reasons:
// 1. It's a separate bit in CPUID, so a processor could implement SSE4.2 and
//    not POPCNT, but that's unlikely to happen.
// 2. There are processors that support POPCNT but not AVX (Intel Nehalem
//    architecture), but unlike the other compilers, MSVC has no option
//    to generate code for those processors.
// So it's an acceptable compromise.
#if defined(__AVX__) || defined(__SSE4_2__) || defined(__POPCNT__)
#define SC_ALGORITHMS_USE_BUILTIN_POPCOUNT
#define SC_ALGORITHMS_USE_BUILTIN_POPCOUNTLL
SC_ALWAYS_INLINE scuint32 builtin_popcount(scuint32 v) noexcept {
    return __popcnt(v);
}

SC_ALWAYS_INLINE scuint32 builtin_popcount(quint8 v) noexcept {
    return __popcnt16(v);
}

SC_ALWAYS_INLINE scuint32 builtin_popcount(scuint16 v) noexcept {
    return __popcnt16(v);
}

SC_ALWAYS_INLINE scuint32 builtin_popcountll(scuint64 v) noexcept 
{
#if SC_PROCESSOR_WORDSIZE == 8
    return __popcnt64(v);
#else
    return __popcnt(scuint32(v)) + __popcnt(scuint32(v >> 32));
#endif // MSVC 64bit
}

#endif // __AVX__ || __SSE4_2__ || __POPCNT__

#endif // MSVC
#endif // SC_HAS_CONSTEXPR_BUILTINS

SC_END_NAMESPACE

#endif