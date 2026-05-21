#ifndef SCFREELIST
#define SCFREELIST

#include "scutils/scglobal.h"
#include "scutils/thread/scatomic.h"

template <typename T>
struct ScFreeListElement
{
    typedef const T& ConstReferenceType;
    typedef T& ReferenceType;

    T _t;
    ScAtomicInt next;

    inline ConstReferenceType t() const { return _t; }
    inline ReferenceType t() { return _t; }
};

template <>
struct ScFreeListElement<void>
{
    typedef void ConstReferenceType;
    typedef void ReferenceType;

    ScAtomicInt next;

    inline void t() const {}
    inline void t() {}
};

struct ScFreeListDefaultConstants
{
    enum
    {
        InitialNextValue = 0,
        IndexMask = 0x00ffffff,
        SerialMask = ~IndexMask & ~0x80000000,
        SerialCounter = IndexMask + 1,
        MaxIndex = IndexMask,
        BlockCount = 4
    };

    static const int Sizes[BlockCount];
};

/*! \internal

    This is a generic implementation of a lock-free free list. Use next() to
    get the next free entry in the list, and release(id) when done with the id.

    This version is templated and allows having a payload of type T which can
    be accessed using the id returned by next(). The payload is allocated and
    deallocated automatically by the free list, but *NOT* when calling
    next()/release(). Initialization should be done by code needing it after
    next() returns. Likewise, cleanup() should happen before calling release().
    It is possible to have use 'void' as the payload type, in which case the
    free list only contains indexes to the next free entry.

    The ConstantsType type defaults to ScFreeListDefaultConstants above. You can
    define your custom ConstantsType, see above for details on what needs to be
    available.
*/
template <typename T, typename ConstantsType = ScFreeListDefaultConstants>
class ScFreeList
{
    typedef T ValueType;
    typedef ScFreeListElement<T> ElementType;
    typedef typename ElementType::ConstReferenceType ConstReferenceType;
    typedef typename ElementType::ReferenceType ReferenceType;

    // return which block the index \a x falls in, and modify \a x to be the index into that block
    static inline int blockfor(int& x)
    {
        for (int i = 0; i < ConstantsType::BlockCount; ++i)
        {
            int size = ConstantsType::Sizes[i];
            if (x < size)
                return i;
            x -= size;
        }
        SC_ASSERT(false);
        return -1;
    }

    // allocate a block of the given \a size, initialized starting with the given \a offset
    static inline ElementType* allocate(int offset, int size)
    {
        // qDebug("ScFreeList: allocating %d elements (%ld bytes) with offset %d", size, size * sizeof(ElementType), offset);
        ElementType* v = new ElementType[size];
        for (int i = 0; i < size; ++i)
            v[i].next.storeRelaxed(offset + i + 1);
        return v;
    }

    // take the current serial number from \a o, increment it, and store it in \a n
    static inline int incrementserial(int o, int n)
    {
        return int((scuint32(n) & ConstantsType::IndexMask) | ((scuint32(o) + ConstantsType::SerialCounter) & ConstantsType::SerialMask));
    }

    // the blocks
    ScAtomicPointer<ElementType> _v[ConstantsType::BlockCount];
    // the next free id
    ScAtomicInt _next;

    // ScFreeList is not copyable
    SC_DISABLE_COPY_MOVE(ScFreeList)

public:
    SC_DECL_CONSTEXPR inline ScFreeList();
    inline ~ScFreeList();

    // returns the payload for the given index \a x
    inline ConstReferenceType at(int x) const;
    inline ReferenceType operator[](int x);

    /*
        Return the next free id. Use this id to access the payload (see above).
        Call release(id) when done using the id.
    */
    inline int next();
    inline void release(int id);
};

template <typename T, typename ConstantsType>
SC_DECL_CONSTEXPR inline ScFreeList<T, ConstantsType>::ScFreeList()
    :
#if defined(SC_COMPILER_CONSTEXPR)
    _v{}, // uniform initialization required
#endif
    _next(ConstantsType::InitialNextValue)
{
}

template <typename T, typename ConstantsType>
inline ScFreeList<T, ConstantsType>::~ScFreeList()
{
    for (int i = 0; i < ConstantsType::BlockCount; ++i)
        delete[] _v[i].loadAcquire();
}

template <typename T, typename ConstantsType>
inline typename ScFreeList<T, ConstantsType>::ConstReferenceType ScFreeList<T, ConstantsType>::at(int x) const
{
    const int block = blockfor(x);
    return (_v[block].loadRelaxed())[x].t();
}

template <typename T, typename ConstantsType>
inline typename ScFreeList<T, ConstantsType>::ReferenceType ScFreeList<T, ConstantsType>::operator[](int x)
{
    const int block = blockfor(x);
    return (_v[block].loadRelaxed())[x].t();
}

template <typename T, typename ConstantsType>
inline int ScFreeList<T, ConstantsType>::next()
{
    int id, newid, at;
    ElementType* v;
    do {
        id = _next.loadAcquire();

        at = id & ConstantsType::IndexMask;
        const int block = blockfor(at);
        v = _v[block].loadAcquire();

        if (!v)
        {
            v = allocate((id & ConstantsType::IndexMask) - at, ConstantsType::Sizes[block]);
            if (!_v[block].testAndSetRelease(nullptr, v))
            {
                // race with another thread lost
                delete[] v;
                v = _v[block].loadAcquire();
                SC_ASSERT(v != nullptr);
            }
        }

        newid = v[at].next.loadRelaxed() | (id & ~ConstantsType::IndexMask);
    } while (!_next.testAndSetRelease(id, newid));
    return id & ConstantsType::IndexMask;
}

template <typename T, typename ConstantsType>
inline void ScFreeList<T, ConstantsType>::release(int id)
{
    int at = id & ConstantsType::IndexMask;
    const int block = blockfor(at);
    ElementType* v = _v[block].loadRelaxed();

    int x, newid;
    do {
        x = _next.loadAcquire();
        v[at].next.storeRelaxed(x & ConstantsType::IndexMask);

        newid = incrementserial(x, id);
    } while (!_next.testAndSetRelease(x, newid));
}

#endif // SCFREELIST
