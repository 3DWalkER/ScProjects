#ifndef SCARRAYDATA_H
#define SCARRAYDATA_H

#include "scutils/utils/screfcount.h"
#include "scutils/utils/scflags.h"

struct SC_API_EXPORT ScArrayData
{
	ScRefCount ref;
	int size;
	scint32 alloc : 31;
	scint32 capacityReserved : 1;

	scptrdiff offset;

	enum AllocOption
	{
		CapacityReserved = 0x01,
		Unsharable = 0x2,
		RawData = 0x4,
		Grow = 0x8,
		Default = 0
	};
	SC_DECLARE_FLAGS(AllocOptions, AllocOption)

	void* data() {
		SC_ASSERT(0 == size || offset < 0 || sizeof(offset) >= sizeof(ScArrayData));
		return reinterpret_cast<char*>(this) + offset;
	}

	const void* data() const {
		SC_ASSERT(0 == size || offset < 0 || sizeof(offset) >= sizeof(ScArrayData));
		return reinterpret_cast<const char*>(this) + offset;
	}

	bool isMutable() const { return 0 != alloc; }

	size_t detachCapacity(size_t newSize) const
	{
		if (capacityReserved && newSize < alloc)
			return alloc;
		return newSize;
	}

	AllocOptions detachFlags() const
	{
		AllocOptions result;
		if (capacityReserved)
			result |= CapacityReserved;
		return result;
	}

	AllocOptions cloneFlags() const
	{
		AllocOptions result;
		if (capacityReserved)
			result |= CapacityReserved;
		return result;
	}

	SC_REQUIRED_RESULT static ScArrayData* allocate(size_t objSize, size_t alignment, size_t capacity, AllocOptions options = Default) noexcept;
	SC_REQUIRED_RESULT static ScArrayData* reallocateUnaligned(ScArrayData* data, size_t objectSize,
		size_t newCapacity, AllocOptions newOptions = Default) noexcept;
	static void deallocate(ScArrayData* data, size_t objectSize, size_t alignment) noexcept;

	static const ScArrayData shared_null[2];
	static ScArrayData* sharedNull() noexcept { return const_cast<ScArrayData*>(shared_null); }
};

SC_DECLARE_OPERATORS_FOR_FLAGS(ScArrayData::AllocOptions)

template <class T>
struct ScTypedArrayData : ScArrayData
{
#ifdef SC_STRICT_ITERATORS
    class iterator
    {
    public:
        T* i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef int difference_type;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;

        inline iterator() : i(nullptr) {}
        inline iterator(T* n) : i(n) {}
        inline iterator(const iterator& o) : i(o.i) {}
        inline T& operator*() const { return *i; }
        inline T* operator->() const { return i; }
        inline T& operator[](int j) const { return *(i + j); }
        inline bool operator==(const iterator& o) const { return i == o.i; }
        inline bool operator!=(const iterator& o) const { return i != o.i; }
        inline bool operator<(const iterator& other) const { return i < other.i; }
        inline bool operator<=(const iterator& other) const { return i <= other.i; }
        inline bool operator>(const iterator& other) const { return i > other.i; }
        inline bool operator>=(const iterator& other) const { return i >= other.i; }
        inline iterator& operator++() { ++i; return *this; }
        inline iterator operator++(int) { T* n = i; ++i; return n; }
        inline iterator& operator--() { i--; return *this; }
        inline iterator operator--(int) { T* n = i; i--; return n; }
        inline iterator& operator+=(int j) { i += j; return *this; }
        inline iterator& operator-=(int j) { i -= j; return *this; }
        inline iterator operator+(int j) const { return iterator(i + j); }
        inline iterator operator-(int j) const { return iterator(i - j); }
        friend inline iterator operator+(int j, iterator k) { return k + j; }
        inline int operator-(iterator j) const { return i - j.i; }
        inline operator T* () const { return i; }
    };
    friend class iterator;

    class const_iterator
    {
    public:
        const T* i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef int difference_type;
        typedef T value_type;
        typedef const T* pointer;
        typedef const T& reference;

        inline const_iterator() : i(nullptr) {}
        inline const_iterator(const T* n) : i(n) {}
        inline const_iterator(const const_iterator& o) : i(o.i) {}
        inline explicit const_iterator(const iterator& o) : i(o.i) {}
        inline const T& operator*() const { return *i; }
        inline const T* operator->() const { return i; }
        inline const T& operator[](int j) const { return *(i + j); }
        inline bool operator==(const const_iterator& o) const { return i == o.i; }
        inline bool operator!=(const const_iterator& o) const { return i != o.i; }
        inline bool operator<(const const_iterator& other) const { return i < other.i; }
        inline bool operator<=(const const_iterator& other) const { return i <= other.i; }
        inline bool operator>(const const_iterator& other) const { return i > other.i; }
        inline bool operator>=(const const_iterator& other) const { return i >= other.i; }
        inline const_iterator& operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const T* n = i; ++i; return n; }
        inline const_iterator& operator--() { i--; return *this; }
        inline const_iterator operator--(int) { const T* n = i; i--; return n; }
        inline const_iterator& operator+=(int j) { i += j; return *this; }
        inline const_iterator& operator-=(int j) { i -= j; return *this; }
        inline const_iterator operator+(int j) const { return const_iterator(i + j); }
        inline const_iterator operator-(int j) const { return const_iterator(i - j); }
        friend inline const_iterator operator+(int j, const_iterator k) { return k + j; }
        inline int operator-(const_iterator j) const { return i - j.i; }
        inline operator const T* () const { return i; }
    };
    friend class const_iterator;
#else
	typedef T* iterator;
	typedef const T* const_iterator;
#endif
	T* data() { return static_cast<T*>(ScArrayData::data()); }
	const T* data() const { return static_cast<const T*>(ScArrayData::data()); }

	iterator begin(iterator = iterator()) { return data(); }
	iterator end(iterator = iterator()) { return data() + size; }
	const_iterator begin(const_iterator = const_iterator()) const { return data(); }
	const_iterator end(const_iterator = const_iterator()) const { return data() + size; }
	const_iterator constBegin(const_iterator = const_iterator()) const { return data(); }
	const_iterator constEnd(const_iterator = const_iterator()) const { return data() + size; }

    class AlignmentDummy { ScArrayData header; T data; };

    SC_REQUIRED_RESULT static ScTypedArrayData* allocate(size_t capacity, AllocOptions options = Default)
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        return static_cast<ScTypedArrayData*>(ScArrayData::allocate(sizeof(T), SC_ALIGNOF(AlignmentDummy), capacity, options));
    }

    static ScTypedArrayData* reallocateUnaligned(ScTypedArrayData* data, size_t capacity, AllocOptions options = Default)
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        return static_cast<ScTypedArrayData*>(ScArrayData::reallocateUnaligned(data, sizeof(T), capacity, options));
    }

    static void deallocate(ScArrayData* data) {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        ScArrayData::deallocate(data, sizeof(T), Q_ALIGNOF(AlignmentDummy));
    }

    static ScTypedArrayData* fromRawData(const T* data, size_t n, AllocOptions options = Default)
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        ScTypedArrayData* result = allocate(0, options | RawData);
        if (result)
        {
            SC_ASSERT(!result->ref.isShared());
            result->offset = reinterpret_cast<const char*>(data) - reinterpret_cast<const char*>(result);
            result->size = int(n);
        }
        return result;
    }

    static ScTypedArrayData* sharedNull() noexcept
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        return static_cast<ScTypedArrayData*>(ScArrayData::sharedNull());
    }

    static ScTypedArrayData* sharedEmpty()
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        return allocate(0);
    }

    static ScTypedArrayData* unsharableEmpty()
    {
        SC_STATIC_ASSERT(sizeof(ScTypedArrayData) == sizeof(ScArrayData));
        return allocate(0, Unsharable);
    }
};

#endif // SCARRAYDATA_H