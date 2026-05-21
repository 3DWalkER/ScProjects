#ifndef SCGLOBAL_H
#define SCGLOBAL_H

#include "scutils/utils/scsystemdetection.h"
#include "scutils/utils/sccppattributes.h"
#include "scutils/utils/scprocessordetection.h"

#if defined(SC_API_EXPORTS)
#	define SC_API_EXPORT SC_DECL_EXPORT
#else
#	define SC_API_EXPORT SC_DECL_IMPORT
#endif

#define SC_UNUSED(x) (void)x;

#if defined(__cplusplus) && defined(SC_COMPILER_STATIC_ASSERT)
#	define SC_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
#	define SC_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
#elif defined(SC_COMPILER_STATIC_ASSERT)
// C11 mode - using the _S version in case <assert.h> doesn't do the right thing
#	define SC_STATIC_ASSERT(Condition) _Static_assert(!!(Condition), #Condition)
#	define SC_STATIC_ASSERT_X(Condition, Message) _Static_assert(!!(Condition), Message)
#else
// C89 & C99 version
#	define SC_STATIC_ASSERT_PRIVATE_JOIN(A, B) SC_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B)
#	define SC_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B) A ## B
#	ifdef __COUNTER__
#		define SC_STATIC_ASSERT(Condition) \
			typedef char SC_STATIC_ASSERT_PRIVATE_JOIN(q_static_assert_result, __COUNTER__) [(Condition) ? 1 : -1];
#  else
#		define SC_STATIC_ASSERT(Condition) \
			typedef char SC_STATIC_ASSERT_PRIVATE_JOIN(q_static_assert_result, __LINE__) [(Condition) ? 1 : -1];
#  endif /* __COUNTER__ */
#	define SC_STATIC_ASSERT_X(Condition, Message) SC_STATIC_ASSERT(Condition)
#endif

template <typename T> inline T* ScGetPtrHelper(T* ptr) { return ptr; }
template <typename Ptr> inline auto ScGetPtrHelper(Ptr& ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }

#define SC_CAST_IGNORE_ALIGN(body) SC_WARNING_PUSH SC_WARNING_DISABLE_GCC("-Wcast-align") body SC_WARNING_POP

#define SC_DECLARE_PRIVATE(Class) \
    inline Class##Private *d_func() \
    { SC_CAST_IGNORE_ALIGN(return reinterpret_cast<Class##Private *>(ScGetPtrHelper(d_ptr));) } \
    inline const Class##Private* d_func() const \
    { SC_CAST_IGNORE_ALIGN(return reinterpret_cast<const Class##Private *>(ScGetPtrHelper(d_ptr));) } \
    friend class Class##Private;

#define SC_DECLARE_PUBLIC(Class) \
    inline Class *q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class *q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define SC_D(Class) Class##Private * const d = d_func()
#define SC_Q(Class) Class * const q = q_func()

#define SC_BEGIN_NAMESPACE namespace Sc {
#define SC_END_NAMESPACE }
#define SC_BEGIN_DETAIL_NAMESPACE namespace detail {
#define SC_END_DETAIL_NAMESPACE SC_END_NAMESPACE
#define SC_DETAIL Sc::detail

#define SC_SAVE_DELETE(ptr) \
    if (nullptr != ptr) { delete ptr; ptr = nullptr; }

#define SC_SAVE_DELETE_ARRAY(ptr) \
    if (nullptr != ptr) { delete[] ptr; ptr = nullptr; }

#define SC_SAVE_FREE(ptr) \
    if (nullptr != ptr) { free(ptr); ptr = nullptr; }

#define SC_DECLARE_SINGLETON(Cls)									\
    public:															\
		static Cls *instance();										\
		static void destroy();										\
    private:														\
		static Cls *m_pInstance;

#define SC_DEFINE_SIGNLETON_LAZY(Cls)								\
    Cls *Cls::m_pInstance = new Cls();								\
																	\
	void Cls::destroy()												\
	{																\
		SC_SAVE_DELETE(m_pInstance);								\
    }																\
																	\
    Cls *Cls::instance()											\
	{																\
		return m_pInstance;											\
    }

#define SC_DEFINE_SIGNLETON_HUNGRY(Cls, ...)						\
    _SC_DEFINE_SIGNALETON(Cls);										\
																	\
    Cls *Cls::instance()											\
	{																\
		if (nullptr == m_pInstance) \
		m_pInstance = new Cls(##__VA_ARGS__);						\
																	\
		return m_pInstance;											\
    }

#define SC_DEFINE_SIGNLETON_SAVE(Cls) 								\
    std::mutex Cls##_instance_mutex; 								\
    																\
    _SC_DEFINE_SIGNALETON(Cls); 									\
    																\
    Cls *Cls::instance() 											\
	{ 																\
		std::lock_guard<std::mutex> lock(Cls##_instance_mutex);		\
		if (nullptr == m_pInstance) 								\
		m_pInstance = new Cls(); 									\
																	\
		return m_pInstance; 										\
    }

#define SC_DEFINE_SIGNLETON_SAVE_FAST(Cls)							\
    std::mutex Cls##_instance_mutex;								\
																	\
    _SC_DEFINE_SIGNALETON(Cls);										\
    																\
    Cls *Cls::instance() 											\
	{ 																\
		if (nullptr == m_pInstance) 								\
		{ 															\
			std::lock_guard<std::mutex> lock(Cls##_instance_mutex); \
			if (nullptr == m_pInstance) 							\
			m_pInstance = new Cls();								\
		}															\
		return m_pInstance; 										\
    }

#define _SC_DEFINE_SIGNALETON(Cls)									\
	Cls *Cls::m_pInstance = nullptr;								\
																	\
	void Cls::destroy()												\
	{																\
		SC_SAVE_DELETE(m_pInstance);								\
    }

#ifndef SC_CONSTRUCTOR_FUNCTION
#	define __SC_CONSTRUCTOR_FUNCTION__(AFUNC)					\
		namespace {												\
			static const struct AFUNC ## _ctor_class_ {			\
				inline AFUNC ## _ctor_class_() { AFUNC(); }		\
			} AFUNC ## _ctor_instance_;							\
		}
#	define SC_CONSTRUCTOR_FUNCTION(AFUNC) __SC_CONSTRUCTOR_FUNCTION__(AFUNC)
#endif

#ifndef SC_DESTRUCTOR_FUNCTION
#	define __SC_DESTRUCTOR_FUNCTION__(AFUNC)					\
		namespace {												\
			static const struct AFUNC ## _dtor_class_ {			\
				inline AFUNC ## _dtor_class_() { }				\
				inline ~AFUNC ## _dtor_class_() { AFUNC(); }	\
			} AFUNC ## _dtor_instance_;							\
		}
#	define SC_DESTRUCTOR_FUNCTION(AFUNC) __SC_DESTRUCTOR_FUNCTION__(AFUNC)
#endif

#define SC_DISABLE_COPY(Class)						\
    Class(const Class &) = delete;					\
    Class &operator=(const Class &) = delete;

#define SC_DISABLE_MOVE(Class)						\
    Class(Class &&) = delete;						\
    Class &operator=(Class &&) = delete;

#define SC_DISABLE_COPY_MOVE(Class)					\
    SC_DISABLE_COPY(Class)							\
    SC_DISABLE_MOVE(Class)

typedef signed char scint8;
typedef unsigned char scuint8;
typedef short scint16;
typedef unsigned short scuint16;
typedef int scint32;
typedef unsigned int scuint32;
#if defined(SC_OS_WIN) && !defined(SC_CC_GNU)
typedef __int64 scint64;
typedef unsigned __int64 scuint64;
#	define SC_INT64_C(c) c ## i64	/* signed 64 bit constant */
#	define SC_UINT64_C(c) c ## ui64	/* signed 64 bit constant */
#else
#	ifdef __cplusplus
#		define SC_INT64_C(c) static_cast<long long>(c ## LL)
#		define SC_UINT64_C(c) static_cast<unsigned long long>(c ## ULL)
#	else
#		define SC_INT64_C(c) ((long long)(c ## LL))
#		define SC_UINT64_C(c) ((unsigned long long)(c ## ULL))
#	endif
typedef long long scint64;
typedef unsigned long long scuint64;
#endif

typedef scuint64 scid;

template <int> struct ScIntegerForSize;
template <>    struct ScIntegerForSize<1> { typedef scuint8  Unsigned; typedef scint8  Signed; };
template <>    struct ScIntegerForSize<2> { typedef scuint16 Unsigned; typedef scint16 Signed; };
template <>    struct ScIntegerForSize<4> { typedef scuint32 Unsigned; typedef scint32 Signed; };
template <>    struct ScIntegerForSize<8> { typedef scuint64 Unsigned; typedef scint64 Signed; };
#if defined(SC_CC_GNU) && defined(__SIZEOF_INT128__)
template <>    struct ScIntegerForSize<16> { __extension__ typedef unsigned __int128 Unsigned; __extension__ typedef __int128 Signed; };
#endif
template <class T> struct ScIntegerForSizeof : ScIntegerForSize<sizeof(T)> {};
typedef ScIntegerForSize<SC_PROCESSOR_WORDSIZE>::Signed scregisterint;
typedef ScIntegerForSize<SC_PROCESSOR_WORDSIZE>::Unsigned scregisteruint;
typedef ScIntegerForSizeof<void*>::Unsigned scuintptr;
typedef ScIntegerForSizeof<void*>::Signed scptrdiff;
typedef scptrdiff scintptr;
using scsizetype = ScIntegerForSizeof<std::size_t>::Signed;

template <typename T>
SC_DECL_CONSTEXPR typename std::add_const<T>::type& scAsConst(T& t) noexcept { return t; }

template <typename T>
void scAsConst(const T&&) = delete;

template <typename T>
constexpr inline const T& scMin(const T& a, const T& b) { return (a < b) ? a : b; }
template <typename T>
constexpr inline const T& scMax(const T& a, const T& b) { return (a < b) ? b : a; }
template <typename T>
constexpr inline const T& scBound(const T& min, const T& val, const T& max) { return scMax(min, scMin(max, val)); }

class ScString;

#if !(defined(_DEBUG) || !defined(NDEBUG))
#	define SC_NO_DEBUG
#endif

SC_DECL_COLD_FUNCTION
SC_API_EXPORT void sc_assert(const char* assertion, const char* file, int line) noexcept;

#if !defined(SC_ASSERT)
#	if defined(SC_NO_DEBUG)
#		define SC_ASSERT(cond) static_cast<void>(false && (cond))
#	else
#		define SC_ASSERT(cond) ((cond) ? static_cast<void>(0) : sc_assert(#cond, __FILE__, __LINE__))
#	endif
#endif

SC_DECL_COLD_FUNCTION
SC_API_EXPORT void sc_assert_x(const char* where, const char* what, const char* file, int line) noexcept;

#if !defined(SC_ASSERT_X)
#	if defined(SC_NO_DEBUG)
#		define SC_ASSERT_X(cond, where, what) static_cast<void>(false && (cond))
#	else
#		define SC_ASSERT_X(cond, where, what) ((cond) ? static_cast<void>(0) : sc_assert_x(where, what, __FILE__, __LINE__))
#	endif
#endif

#endif // SCGLOBAL_H
