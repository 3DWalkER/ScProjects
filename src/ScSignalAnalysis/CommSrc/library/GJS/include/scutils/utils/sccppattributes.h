#ifndef SCGLOBAL_H
#   include "scutils/scglobal.h"
#endif

#ifndef SCCPPATTRIBUTES_H
#define SCCPPATTRIBUTES_H

#include "scutils/utils/sccompilerdetection.h"

#if defined(SC_CC_MSVC)
#	define SC_CPLUSPLUS _MSVC_LANG
#else
#	define SC_CPLUSPLUS __cplusplus
#endif

#ifdef SC_CC_MSVC
#	define SC_NEVER_INLINE	__declspec(noinline)
#	define SC_ALWAYS_INLINE __forceinline
#elif defined(SC_CC_GNU)
#	define SC_NEVER_INLINE	__attribute__((noinline))
#	define SC_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#	define SC_NEVER_INLINE
#	define SC_ALWAYS_INLINE inline
#endif

#if defined(SC_CC_GNU)
#	define SC_EXPORT __attribute__((__visibility__("default")))
#else
#	define SC_EXPORT
#endif

#if SC_HAS_CPP_ATTRIBUTE(gnu::cold)
#	define SC_ATTR_GUN_COLD [[gnu::cold]]
#else
#	define SC_ATTR_GUN_COLD
#endif

#if SC_HAS_CPP_ATTRIBUTE(maybe_unused)
#   define SC_ATTR_MAYBE_UNUSED [[maybe_unused]]
#else
#   define SC_ATTR_MAYBE_UNUSED
#endif

#endif // SCCPPATTRIBUTES_H