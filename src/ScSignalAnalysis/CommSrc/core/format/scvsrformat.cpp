#include "scvsrformat.h"

#include "scutils/utils/scprocessordetection.h"
#include <vector>

template <int BitSize, typename T>
struct CDTableBit {
	static const std::vector<T> table;
};

template <typename T>
using CDFactorType = typename std::conditional<std::is_integral<T>::value, double, T>::type;

template <typename T, int N>
constexpr std::vector<T> makeTable(int bitWidth, CDFactorType<T> factor)
{
	std::vector<T> t(N);
	const unsigned int sign = 1u << (bitWidth - 1);
	for (int i = 0; i < N; i++)
	{
		int k = static_cast<int>((i ^ sign) - sign);
		t[i] = static_cast<T>(((k << 1) + 1) / factor);
	}
	return t;
}

/** *************************************************< 1比特 >************************************************* **/
template <> const std::vector<short> CDTableBit<1, short>::table = { 1000 , -1000 };
template <> const std::vector<float> CDTableBit<1, float>::table = { 1.0f , -1.0f };
template <> const std::vector<double> CDTableBit<1, double>::table = { 1.0  , -1.0 };

template <typename T>
static inline void cdbswap_vsr1(const char* pRaw, T* pDst)
{
	const T* pTarget = CDTableBit<1, T>::table.data();

	const unsigned char temp = pRaw[0];
	*pDst++ = (temp & 0x01) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x02) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x04) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x08) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x10) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x20) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x40) ? pTarget[0] : pTarget[1];
	*pDst++ = (temp & 0x80) ? pTarget[0] : pTarget[1];
}

static inline void cdbswap_vsr1_short(const char* pRaw, char* pDst) {
	return cdbswap_vsr1(pRaw, reinterpret_cast<short*>(pDst));
}

/** *************************************************< 2比特 >************************************************* **/
template <> const std::vector<short> CDTableBit<2, short>::table = { 4000, 12000, -12000, -4000 };
template <> const std::vector<float> CDTableBit<2, float>::table = { 1.0f, 3.0f, -3.0f, -1.0f };
template <> const std::vector<double> CDTableBit<2, double>::table = { 1.0, 3.0, -3.0, -1.0 };

template <typename T>
static inline void cdbswap_vsr2(const char* pRaw, T* pDst)
{
	const T* pTarget = CDTableBit<2, T>::table.data();
	const unsigned int temp = static_cast<unsigned char>(pRaw[0]);
	*pDst++ = pTarget[temp & 0x03];
	*pDst++ = pTarget[(temp >> 2) & 0x03];
	*pDst++ = pTarget[(temp >> 4) & 0x03];
	*pDst++ = pTarget[temp >> 6];
}

static inline void cdbswap_vsr2_short(const char* pRaw, char* pDst) {
	return cdbswap_vsr2(pRaw, reinterpret_cast<short*>(pDst));
}

/** *************************************************< 4比特 >************************************************* **/
template <> const std::vector<short> CDTableBit<4, short>::table = makeTable<short, 16>(4, 0.001);
template <> const std::vector<float> CDTableBit<4, float>::table = makeTable<float, 16>(4, 15.0f);
template <> const std::vector<double> CDTableBit<4, double>::table = makeTable<double, 16>(4, 15.0);

template <typename T>
static inline void cdbswap_vsr4(const char* pRaw, T* pDst)
{
	const unsigned int temp = static_cast<unsigned char>(pRaw[0]);
	*pDst++ = CDTableBit<4, T>::table[temp & 15];
	*pDst++ = CDTableBit<4, T>::table[temp >> 4];
}

static inline void cdbswap_vsr4_short(const char* pRaw, char* pDst) {
	return cdbswap_vsr4(pRaw, reinterpret_cast<short*>(pDst));
}

/** *************************************************< 8比特 >************************************************* **/
template <> const std::vector<short> CDTableBit<8, short>::table = makeTable<short, 256>(8, 0.02);
template <> const std::vector<float> CDTableBit<8, float>::table = makeTable<float, 256>(8, 255.0f);
template <> const std::vector<double> CDTableBit<8, double>::table = makeTable<double, 256>(8, 255.0);

template <typename T>
static inline void cdbswap_vsr8(const char* pRaw, T* pDst) {
	pDst[0] = CDTableBit<8, T>::table[static_cast<unsigned char>(pRaw[0])];
}

static inline void cdbswap_vsr8_short(const char* pRaw, char* pDst) {
	return cdbswap_vsr8(pRaw, reinterpret_cast<short*>(pDst));
}

/** ************************************************< 16比特 >************************************************* **/
template <> const std::vector<short> CDTableBit<16, short>::table = makeTable<short, 65536>(16, 2);
template <> const std::vector<float> CDTableBit<16, float>::table = makeTable<float, 65536>(16, 65536.0f);
template <> const std::vector<double> CDTableBit<16, double>::table = makeTable<double, 65536>(16, 65536.0);

template <typename T>
static inline void cdbswap_vsr16(const char* pRaw, T* pDst) {
	pDst[0] = CDTableBit<16, T>::table[*reinterpret_cast<const unsigned short*>(pRaw)];
}

static inline void cdbswap_vsr16_short(const char* pRaw, char* pDst) {
	return cdbswap_vsr16(pRaw, reinterpret_cast<short*>(pDst));
}

ScVsrFormat::ScVsrFormat()
	: ScSignalFormat(Sc::DF_VSR, Sc::DBM_Multiple, Sc::CM_PerByte)
{
	
}

bool ScVsrFormat::isValid(const char* inBuff, int inlen)
{
	if (!inBuff || inlen < sizeof(VSR_HEADER))
		return false;

	const VSR_HEADER* pHeader = reinterpret_cast<const VSR_HEADER*>(inBuff);
	const unsigned int label = *reinterpret_cast<const unsigned int*>(pHeader->record_label);
#if SC_LITTLE_ENDIAN == SC_BYTE_ORDER
	bool isOk = 0b01000110010001010100010001010010 == label;
#else
	bool isOk = 0b10010100010001000100010101000100 == label;
#endif
	if (isOk)
	{
		m_blockSize = pHeader->record_length - sizeof(VSR_HEADER);
		m_sampleRate = static_cast<double>(pHeader->sample_rate);
		m_isIQ = true;
		setBitDepth(pHeader->sample_size);
	}
	return isOk;
}
