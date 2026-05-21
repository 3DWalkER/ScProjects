#ifndef SCENDIAN_H
#define SCENDIAN_H

#include "scutils/scglobal.h"
#include <string.h>

SC_BEGIN_NAMESPACE

/**
 * @brief isLittleEndian 判断系统是否为小端存储
 */
inline bool SC_API_EXPORT isLittleEndian() {
	const unsigned short val = 0x0102;
	return 0x02 == *reinterpret_cast<const unsigned char*>(&val);
}

/**
 * @brief isBigEndian 判断系统是否为大端存储
 */
inline bool SC_API_EXPORT isBigEndian() { return !isLittleEndian(); }

template <typename T> SC_ALWAYS_INLINE void toUnaligned(const T src, void* dest)
{
	const size_t size = sizeof(T);
	memcpy(dest, &src, size);
}

template <typename T> SC_ALWAYS_INLINE T fromUnaligned(const void* src)
{
	T dest;
	const size_t size = sizeof(T);
	memcpy(&dest, src, size);
	return dest;
}

/**
 * @brief endianSwap 大小端转换
 */
template <typename T> constexpr T endianSwap(T source);

template <> inline constexpr scuint64 endianSwap<scuint64>(scuint64 source)
{
	return 0
		| ((source & (0x00000000000000ff)) << 56)
		| ((source & (0x000000000000ff00)) << 40)
		| ((source & (0x0000000000ff0000)) << 24)
		| ((source & (0x00000000ff000000)) << 8)
		| ((source & (0x000000ff00000000)) >> 8)
		| ((source & (0x0000ff0000000000)) >> 24)
		| ((source & (0x00ff000000000000)) >> 40)
		| ((source & (0xff00000000000000)) >> 56);
}

template <> inline constexpr scuint32 endianSwap<scuint32>(scuint32 source)
{
	return 0
		| ((source & 0x000000ff) << 24)
		| ((source & 0x0000ff00) << 8)
		| ((source & 0x00ff0000) >> 8)
		| ((source & 0xff000000) >> 24);
}

template <> inline constexpr scuint16 endianSwap<scuint16>(scuint16 source)
{
	return scuint16(0
		| ((source & 0x00ff) << 8)
		| ((source & 0xff00) >> 8));
}

template <> inline constexpr scuint8 endianSwap<scuint8>(scuint8 source)
{
	return source;
}

template <> inline constexpr scint64 endianSwap<scint64>(scint64 source) {
	return endianSwap<scuint64>(source);
}

template <> inline constexpr scint32 endianSwap<scint32>(scint32 source) {
	return endianSwap<scuint32>(source);
}

template <> inline constexpr scint16 endianSwap<scint16>(scint16 source) {
	return endianSwap<scuint16>(source);
}

template <> inline constexpr scint8 endianSwap<scint8>(scint8 source) {
	return endianSwap<scuint8>(source);
}

#if SC_LITTLE_ENDIAN == SC_BYTE_ORDER

template <typename T> inline constexpr T toBigEndian(T source) { return endianSwap(source); }
template <typename T> inline constexpr T fromBigEndian(T source) { return endianSwap(source); }
template <typename T> inline constexpr T toLittleEndian(T source) { return source; }
template <typename T> inline constexpr T fromLittleEndian(T source) { return source; }

#else

template <typename T> inline constexpr T toBigEndian(T source) { return source; }
template <typename T> inline constexpr T fromBigEndian(T source) { return source; }
template <typename T> inline constexpr T toLittleEndian(T source) { return endianSwap(source); }
template <typename T> inline constexpr T fromLittleEndian(T source) { return endianSwap(source); }

#endif

template <class T> inline T fromBigEndian(const void* src) { return fromBigEndian(fromUnaligned<T>(src)); }

SC_END_NAMESPACE

#endif // SCENDIAN_H
