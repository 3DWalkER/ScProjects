#ifndef SCMATH_H
#define SCMATH_H

#include "scutils/utils/scalgorithm.h"

SC_BEGIN_NAMESPACE

SC_DECL_RELAXED_CONSTEXPR inline scuint32 nextPowerOfTwo(scuint32 v)
{
#if defined(SC_HAS_BUILTIN_CLZ)
    if (v == 0)
        return 1;
    return 2U << (31 ^ Sc::builtin_clz(v));
#else
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    return v;
#endif
}

SC_DECL_RELAXED_CONSTEXPR inline scuint64 nextPowerOfTwo(scuint64 v)
{
#if defined(SC_HAS_BUILTIN_CLZLL)
    if (v == 0)
        return 1;
    return SC_UINT64_C(2) << (63 ^ Sc::builtin_clzll(v));
#else
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    ++v;
    return v;
#endif
}

SC_DECL_RELAXED_CONSTEXPR inline scuint32 nextPowerOfTwo(scint32 v) {
    return nextPowerOfTwo(scuint32(v));
}

SC_DECL_RELAXED_CONSTEXPR inline scuint64 nextPowerOfTwo(scint64 v) {
    return nextPowerOfTwo(scuint64(v));
}

SC_END_NAMESPACE


#endif // SCMATH_H