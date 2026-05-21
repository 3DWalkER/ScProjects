#ifndef SCSTRINGDATA_H
#define SCSTRINGDATA_H

#include "scutils/utils/scnamespace.h"
#include <string>

typedef char16_t scunicodechar;
SC_STATIC_ASSERT_X(sizeof(scunicodechar) == 2, "scunicodechar must typedef an integral type of size 2");

class SC_API_EXPORT ScStringData
{
public:
	ScStringData() noexcept { reset(); }
	ScStringData(const char* const data, size_t size);
	ScStringData(const ScStringData& other);
	inline ScStringData(ScStringData&& goner) noexcept : ml(goner.ml) { goner.reset(); }
	~ScStringData();

	size_t size() const;
	size_t capacity() const;

	char* data() { return c_str(); }
	const char* data() const { return c_str(); }
	char* mutableData();
	const char* c_str() const {
		return Category::Small == category() ? _small : reinterpret_cast<const char*>(ml.data);
	}

	void swap(ScStringData &rhs);
	void reserve(size_t minCapacity);
	void shrink(size_t delta);
	char* expand(size_t delta, bool isExpGrowth = false);

	bool isShared() const;

	static constexpr auto npos{ static_cast<size_t>(-1) };

private:
	SC_CONSTEXPR static bool kIsLittleEndian = (SC_LITTLE_ENDIAN == SC_BYTE_ORDER);

	enum class Category : scuint8
	{
		Small = 0x0,
		Medium = kIsLittleEndian ? 0x80 : 0x2,
		Large = kIsLittleEndian ? 0x40 : 0x1
	};

	Category category() const {
		return static_cast<Category>(_bytes[kLastChar] & kCategoryMask);
	}

	char* c_str() {
		return Category::Small == category() ? _small : reinterpret_cast<char *>(ml.data);
	}

	void reset() { setSmallSize(0); }
	void reserveSmall(size_t minCapacity);
	void reserveMedium(size_t minCapacity);
	void reserveLarge(size_t minCapacity);
	void unshare(size_t minCapacity = 0);

	int smallSize() const;
	void setSmallSize(int size);

	struct ScMediumLargeString
	{
		char* data{ };
		size_t size{ };
		size_t _capacity{ };

		size_t capacity() const {
			return kIsLittleEndian ? _capacity & kCapacityMask : _capacity >> 2;
		}

		void setCapacity(size_t cap, Category cat) {
			_capacity = kIsLittleEndian
				? cap | (static_cast<size_t>(cat) << kCategoryShift)
				: (cap << 2) | static_cast<size_t>(cat);
		}
	};

	union
	{
		scuint8 _bytes[sizeof(ScMediumLargeString)];
		char _small[sizeof(ScMediumLargeString) / sizeof(char)];
		ScMediumLargeString ml;
	};

	SC_CONSTEXPR static size_t kLastChar = sizeof(ScMediumLargeString) - 1;
	SC_CONSTEXPR static size_t kMaxSmallSize = kLastChar / sizeof(char);
	SC_CONSTEXPR static size_t kMaxMediumSize = 254 / sizeof(char);
	SC_CONSTEXPR static scuint8 kCategoryMask = kIsLittleEndian ? 0xC0 : 0x3;
	SC_CONSTEXPR static size_t kCategoryShift = (sizeof(size_t) - 1) * 8;
	SC_CONSTEXPR static size_t kCapacityMask = kIsLittleEndian ? ~(size_t(kCategoryMask) << kCategoryShift) : 0x0;
};

inline size_t ScStringData::size() const
{
#if SC_LITTLE_ENDIAN == SC_BYTE_ORDER
	typedef typename std::make_unsigned<scint8>::type UChar;
	auto maybeSmallSize = kMaxSmallSize - size_t(static_cast<UChar>(_small[kMaxSmallSize]));
	return static_cast<scptrdiff>(maybeSmallSize) >= 0 ? maybeSmallSize : ml.size;
#else
	return ScStringData::Small == category() ? smallSize() : ml.size;
#endif
}

inline int ScStringData::smallSize() const
{
	SC_ASSERT(Category::Small == category());
	SC_CONSTEXPR auto shift = kIsLittleEndian ? 0 : 2;
	auto smallShifted = static_cast<int>(_small[kMaxSmallSize]) >> shift;
	SC_ASSERT(kMaxSmallSize >= smallShifted);
	return kMaxSmallSize - smallShifted;
}

inline void ScStringData::setSmallSize(int size)
{
	SC_ASSERT(size <= kMaxSmallSize);
	SC_CONSTEXPR auto shift = kIsLittleEndian ? 0 : 2;
	_small[kMaxSmallSize] = scint8((kMaxSmallSize - size) << shift);
	_small[size] = '\0';
	SC_ASSERT(category() == Category::Small && this->size() == size);
}

inline void ScStringData::swap(ScStringData& rhs)
{
	auto const t = ml;
	ml = rhs.ml;
	rhs.ml = t;
}

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

template <class Pod>
inline void podCopy(const Pod* b, const Pod* e, Pod* d)
{
	SC_ASSERT(b != nullptr);
	SC_ASSERT(e != nullptr);
	SC_ASSERT(d != nullptr);
	SC_ASSERT(e >= b);
	SC_ASSERT(d >= e || d + (e - b) <= b);
	memcpy(d, b, (e - b) * sizeof(Pod));
}

template <class Pod>
inline void podMove(const Pod* b, const Pod* e, Pod* d)
{
	SC_ASSERT(b != nullptr);
	SC_ASSERT(e != nullptr);
	SC_ASSERT(d != nullptr);
	SC_ASSERT(e >= b);
	memmove(d, b, (e - b) * sizeof(*b));
}

inline size_t strlen_s(const char* str) {
	return str ? strlen(str) : 0;
}

template <size_t N>
inline void strcpy_s(char(&dst)[N], const char *data, size_t length) {
	size_t copylen = length > N - 1 ? N - 1 : length;
	std::copy(data, data + copylen, dst);
	dst[copylen] = '\0';
}

template <size_t N>
inline void strcpy_s(char(&dst)[N], const ScString& src) {
	SC_DETAIL::strcpy_s(dst, src.data(), src.size());
}

template <size_t N>
inline void strcpy_s(char(&dst)[N], const std::string& src) {
	SC_DETAIL::strcpy_s(dst, src.data(), src.size());
}

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE

#endif // SCSTRINGLITERAL_H