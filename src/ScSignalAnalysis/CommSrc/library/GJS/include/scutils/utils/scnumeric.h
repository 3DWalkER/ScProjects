#ifndef SCNUMERIC_P_H
#define SCNUMERIC_P_H

#include "scutils/scglobal.h"
#include <limits>

SC_BEGIN_NAMESPACE

#if ((defined(SC_CC_INTEL) ? (SC_CC_INTEL >= 1800 && !defined(SC_OS_WIN)) : defined(SC_CC_GNU)) \
     && SC_CC_GNU >= 500) || __has_builtin(__builtin_add_overflow)
template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
add_overflow(T v1, T v2, T* r) {
	return __builtin_add_overflow(v1, v2, r);
}

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
sub_overflow(T v1, T v2, T* r) {
	return __builtin_sub_overflow(v1, v2, r);
}

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
mul_overflow(T v1, T v2, T* r) {
	return __builtin_mul_overflow(v1, v2, r);
}
#else

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
add_overflow(T v1, T v2, T* r) {
	*r = v1 + v2;
	return v1 > T(v1 + v2);
}

template <typename T> inline
typename std::enable_if<std::is_signed<T>::value, bool>::type
add_overflow(T v1, T v2, T* r)
{
	using U = typename std::make_unsigned<T>::type;
	*r = T(U(v1) + U(v2));
	if (std::is_same<int32_t, int>::value)
		return ((v1 ^ *r) & (v2 ^ *r)) < 0;

	bool s1 = (v1 < 0);
	bool s2 = (v2 < 0);
	bool sr = (*r < 0);
	return s1 != sr && s2 != sr;
}

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value, bool>::type
sub_overflow(T v1, T v2, T* r) {
	*r = v1 - v2;
	return v1 < v2;
}

template <typename T> inline
typename std::enable_if<std::is_signed<T>::value, bool>::type
sub_overflow(T v1, T v2, T* r)
{
	using U = typename std::make_unsigned<T>::type;
	*r = T(U(v1) - U(v2));

	if (std::is_same<int32_t, int>::value)
		return ((v1 ^ *r) & (~v2 ^ *r)) < 0;

	bool s1 = (v1 < 0);
	bool s2 = !(v2 < 0);
	bool sr = (*r < 0);
	return s1 != sr && s2 != sr;
}

template <typename T> inline
typename std::enable_if<(std::is_unsigned<T>::value || std::is_signed<T>::value) && sizeof(T) < sizeof(uintmax_t), bool>::type
mul_overflow(T v1, T v2, T* r)
{
	using LargerInt = ScIntegerForSize<sizeof(T) * 2>;
	using Larger = typename std::conditional<std::is_signed<T>::value,
		typename LargerInt::Signed, typename LargerInt::Unsigned>::type;
	Larger lr = Larger(v1) * Larger(v2);
	*r = T(lr);
	return lr > std::numeric_limits<T>::max() || lr < std::numeric_limits<T>::min();
}

SC_BEGIN_DETAIL_NAMESPACE

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) == sizeof(uintmax_t), bool>::type
mul_overflow_generic(T v1, T v2, T* r)
{
	if (0 != v1 && v2 > std::numeric_limits<T>::max() / v1)
	{
		*r = static_cast<T>(v1 * v2);
		return true;
	}
	*r = v1 * v2;
	return false;
}

template <typename T, typename = std::enable_if<std::is_signed<T>::value>::type> inline
typename std::enable_if<sizeof(T) == sizeof(uintmax_t), bool>::type
mul_overflow_generic(T v1, T v2, T* r)
{
	*r = T(v1 * v2);
	const T max = std::numeric_limits<T>::max();
	const T min = std::numeric_limits<T>::min();
	if (v1 > 0 && v2 > 0)
	{
		if (v1 > max / v2)
			return true;
	}
	else if (v1 < 0 && v2 < 0)
	{
		if (v1 < max / v2)
			return true;
	}
	else
	{
		if (0 != v2 && v1 < min / v2)
			return true;
	}
	return false;
}

SC_END_DETAIL_NAMESPACE

template <typename T> inline
typename std::enable_if <(std::is_unsigned<T>::value || std::is_signed<T>::value) && sizeof(T) == sizeof(uintmax_t), bool > ::type
mul_overflow(T v1, T v2, T* r)
{
#if defined(SC_CC_MSVC) && defined(SC_OS_WIN64)
	unsigned __int64 high;
	unsigned __int64 low = _umul128(v1, v2, &high);
	if (SC_LIKELY(0 == high))
	{
		*r = static_cast<T>(low);
		return false;
	}
	*r = {};
	return true;
#else
	SC_DETAIL::mul_overflow_generic(v1, v2, r);
#endif
	return false;
}

#endif

template <typename T> inline typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
muladd_overflow(T v1, T v2, T v3, T* r)
{
	if (mul_overflow(v1, v2, r))
		return true;
	return add_overflow(*r, v3, r);
}

SC_END_NAMESPACE

#endif // SCNUMERIC_P_H