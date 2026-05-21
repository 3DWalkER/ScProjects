#ifndef SCATOMIC_H
#define SCATOMIC_H

#include <atomic>
#include <stddef.h>

template <typename T> struct ScAtomicAdditiveType
{
	typedef T AdditiveT;
	static const int AddScale = 1;
};

template <typename T> struct ScAtomicAdditiveType<T*>
{
	typedef ptrdiff_t AdditiveT;
	static const int AddScale = sizeof(T);
};

template <typename X>
struct ScAtomicOps
{
	typedef std::atomic<X> Type;

	template <typename T>
	static inline T load(const std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_relaxed);
	}

	template <typename T>
	static inline T load(const volatile std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_relaxed);
	}

	template <typename T>
	static inline T loadRelaxed(const std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_relaxed);
	}

	template <typename T>
	static inline T loadRelaxed(const volatile std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_relaxed);
	}

	template <typename T>
	static inline T loadAcquire(const std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_acquire);
	}

	template <typename T>
	static inline T loadAcquire(const volatile std::atomic<T>& value) noexcept {
		return value.load(std::memory_order_acquire);
	}

	template <typename T>
	static inline void store(std::atomic<T>& value, T newValue) noexcept {
		value.store(newValue, std::memory_order_relaxed);
	}

	template <typename T>
	static inline void storeRelaxed(std::atomic<T>& value, T newValue) noexcept {
		value.store(newValue, std::memory_order_relaxed);
	}

	template <typename T>
	static inline void storeRelease(std::atomic<T>& value, T newValue) noexcept {
		value.store(newValue, std::memory_order_release);
	}

	template <typename T>
	static inline bool ref(std::atomic<T>& value) {
		return ++value != 0;
	}

	template <typename T>
	static inline bool deref(std::atomic<T>& value) {
		return --value != 0;
	}

	template <typename T>
	static bool testAndSetRelaxed(std::atomic<T>& value, T expectedValue, T newValue, T* currentValue = nullptr) {
		bool tmp = value.compare_exchange_strong(expectedValue, newValue, std::memory_order_relaxed, std::memory_order_relaxed);
		if (currentValue)
			*currentValue = expectedValue;
		return tmp;
	}

	template <typename T>
	static bool testAndSetAcquire(std::atomic<T>& value, T expectedValue, T newValue, T* currentValue = nullptr) noexcept {
		bool tmp = value.compare_exchange_strong(expectedValue, newValue, std::memory_order_acquire, std::memory_order_acquire);
		if (currentValue)
			*currentValue = expectedValue;
		return tmp;
	}

	template <typename T>
	static bool testAndSetRelease(std::atomic<T>& value, T expectedValue, T newValue, T* currentValue = nullptr) noexcept
	{
		bool tmp = value.compare_exchange_strong(expectedValue, newValue, std::memory_order_release, std::memory_order_relaxed);
		if (currentValue)
			*currentValue = expectedValue;
		return tmp;
	}

	template <typename T>
	static bool testAndSetOrdered(std::atomic<T>& value, T expectedValue, T newValue, T* currentValue = nullptr) noexcept
	{
		bool tmp = value.compare_exchange_strong(expectedValue, newValue, std::memory_order_acq_rel, std::memory_order_acquire);
		if (currentValue)
			*currentValue = expectedValue;
		return tmp;
	}

	template <typename T>
	static inline T fetchAndAddRelaxed(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_add(valueToAdd, std::memory_order_relaxed);
	}

	template <typename T> 
	static inline T fetchAndAddAcquire(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_add(valueToAdd, std::memory_order_acquire);
	}

	template <typename T>
	static inline T fetchAndAddRelease(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_add(valueToAdd, std::memory_order_release);
	}

	template <typename T> 
	static inline T fetchAndAddOrdered(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_add(valueToAdd, std::memory_order_acq_rel);
	}

	template <typename T>
	static inline T fetchAndSubRelaxed(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_sub(valueToAdd, std::memory_order_relaxed);
	}

	template <typename T>
	static inline T fetchAndSubAcquire(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_sub(valueToAdd, std::memory_order_acquire);
	}

	template <typename T>
	static inline T fetchAndSubRelease(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_sub(valueToAdd, std::memory_order_release);
	}

	template <typename T>
	static inline T fetchAndSubOrdered(std::atomic<T>& value, typename ScAtomicAdditiveType<T>::AdditiveT valueToAdd) noexcept
	{
		return value.fetch_sub(valueToAdd, std::memory_order_acq_rel);
	}
};

template <typename T>
class ScBasicAtomicPointer
{
public:
	typedef T* Type;
	typedef ScAtomicOps<Type> Ops;
	typedef typename Ops::Type AtomicType;

	AtomicType _q_value;

	ScBasicAtomicPointer() = default;
	constexpr ScBasicAtomicPointer(Type value) noexcept : _q_value(value) {}

	Type load() const noexcept { return loadRelaxed(); }
	void store(Type newValue) noexcept { storeRelaxed(newValue); }

	Type loadRelaxed() const noexcept { return Ops::loadRelaxed(_q_value); }
	void storeRelaxed(Type newValue) noexcept { Ops::storeRelaxed(_q_value, newValue); }

	Type loadAcquire() const noexcept { return Ops::loadAcquire(_q_value); }
	void storeRelease(Type newValue) noexcept { Ops::storeRelease(_q_value, newValue); }

	bool testAndSetRelaxed(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetRelaxed(_q_value, expectedValue, newValue); }
	bool testAndSetRelaxed(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetRelaxed(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetAcquire(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetAcquire(_q_value, expectedValue, newValue); }
	bool testAndSetAcquire(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetAcquire(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetOrdered(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetOrdered(_q_value, expectedValue, newValue); }
	bool testAndSetOrdered(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetOrdered(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetRelease(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetRelease(_q_value, expectedValue, newValue); }
	bool testAndSetRelease(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetRelease(_q_value, expectedValue, newValue, &currentValue);
	}

	Type fetchAndAddRelaxed(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndAddRelaxed(_q_value, valueToAdd); }
	Type fetchAndAddAcquire(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndAddAcquire(_q_value, valueToAdd); }
	Type fetchAndAddRelease(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndAddRelease(_q_value, valueToAdd); }
	Type fetchAndAddOrdered(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndAddOrdered(_q_value, valueToAdd); }

	Type fetchAndSubRelaxed(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndSubRelaxed(_q_value, valueToAdd); }
	Type fetchAndSubAcquire(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndSubAcquire(_q_value, valueToAdd); }
	Type fetchAndSubRelease(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndSubRelease(_q_value, valueToAdd); }
	Type fetchAndSubOrdered(ptrdiff_t valueToAdd) noexcept { return Ops::fetchAndSubOrdered(_q_value, valueToAdd); }
};

template <typename T>
class ScAtomicPointer : public ScBasicAtomicPointer<T>
{
public:
	constexpr ScAtomicPointer(T* value = nullptr) noexcept : ScBasicAtomicPointer<T>(value) {}
};

template <typename T>
class ScBasicAtomicInteger
{
public:
	typedef T Type;
	typedef ScAtomicOps<Type> Ops;
	typedef typename Ops::Type AtomicType;

	AtomicType _q_value;

	ScBasicAtomicInteger() = default;
	constexpr ScBasicAtomicInteger(Type value) noexcept : _q_value(value) {}

	Type load() const noexcept { return loadRelaxed(); }
	void store(Type newValue) noexcept { storeRelaxed(newValue); }

	Type loadRelaxed() const noexcept { return Ops::loadRelaxed(_q_value); }
	void storeRelaxed(Type newValue) noexcept { Ops::storeRelaxed(_q_value, newValue); }

	Type loadAcquire() const noexcept { return Ops::loadAcquire(_q_value); }
	void storeRelease(T newValue) noexcept { Ops::storeRelease(_q_value, newValue); }

	bool ref() noexcept { return Ops::ref(_q_value); }
	bool deref() noexcept { return Ops::deref(_q_value); }

	bool testAndSetRelaxed(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetRelaxed(_q_value, expectedValue, newValue); }
	bool testAndSetRelaxed(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetRelaxed(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetAcquire(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetAcquire(_q_value, expectedValue, newValue); }
	bool testAndSetAcquire(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetAcquire(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetOrdered(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetOrdered(_q_value, expectedValue, newValue); }
	bool testAndSetOrdered(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetOrdered(_q_value, expectedValue, newValue, &currentValue);
	}

	bool testAndSetRelease(Type expectedValue, Type newValue) noexcept { return Ops::testAndSetRelease(_q_value, expectedValue, newValue); }
	bool testAndSetRelease(Type expectedValue, Type newValue, Type& currentValue) noexcept {
		return Ops::testAndSetRelease(_q_value, expectedValue, newValue, &currentValue);
	}

	T fetchAndAddRelaxed(T valueToAdd) noexcept { return Ops::fetchAndAddRelaxed(_q_value, valueToAdd); }
	T fetchAndAddAcquire(T valueToAdd) noexcept { return Ops::fetchAndAddAcquire(_q_value, valueToAdd); }
	T fetchAndAddRelease(T valueToAdd) noexcept { return Ops::fetchAndAddRelease(_q_value, valueToAdd); }
	T fetchAndAddOrdered(T valueToAdd) noexcept { return Ops::fetchAndAddOrdered(_q_value, valueToAdd); }

	T fetchAndSubRelaxed(T valueToAdd) noexcept { return Ops::fetchAndSubRelaxed(_q_value, valueToAdd); }
	T fetchAndSubAcquire(T valueToAdd) noexcept { return Ops::fetchAndSubAcquire(_q_value, valueToAdd); }
	T fetchAndSubRelease(T valueToAdd) noexcept { return Ops::fetchAndSubRelease(_q_value, valueToAdd); }
	T fetchAndSubOrdered(T valueToAdd) noexcept { return Ops::fetchAndSubOrdered(_q_value, valueToAdd); }
};

typedef ScBasicAtomicInteger<int> ScBasicAtomicInt;

template <typename T>
class ScAtomicInteger : public ScBasicAtomicInteger<T>
{
public:
	constexpr ScAtomicInteger(T value = 0) noexcept : ScBasicAtomicInteger<T>(value) {}

	inline ScAtomicInteger(const ScAtomicInteger& other) noexcept
		: ScBasicAtomicInteger<T>()
	{
		this->storeRelease(other.loadAcquire());
	}

	inline ScAtomicInteger& operator=(const ScAtomicInteger& other) noexcept
	{
		this->storeRelease(other.loadAcquire());
		return *this;
	}
};

class ScAtomicInt : public ScAtomicInteger<int>
{
public:
	constexpr ScAtomicInt(int value = 0) noexcept : ScAtomicInteger<int>(value) {}
};

#endif // SCATOMIC_H