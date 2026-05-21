#ifndef SCEXPLICITLYSHAREDPOINTER_H
#define SCEXPLICITLYSHAREDPOINTER_H

#include <memory>

template <typename T>
class ScExplicitlySharedPointer
{
public:
	inline ScExplicitlySharedPointer() : d(nullptr) {}
	explicit ScExplicitlySharedPointer(T* data) noexcept : d(data) {}
	inline ScExplicitlySharedPointer(const ScExplicitlySharedPointer& o) : d(o.d) {}
	inline ScExplicitlySharedPointer(ScExplicitlySharedPointer&& o) noexcept : d(o.d) { o.d = nullptr; }
	~ScExplicitlySharedPointer() = default;

	inline T* data() const { return d; }
	inline const T* constData() const { return d; }
	inline T* take() { T* x = d; d = nullptr; return x; }

	inline void detach();
	inline void reset() { d = nullptr; }

	inline T& operator*() const { return *d; }
	inline T* operator->() { return d.get(); }
	inline T* operator->() const { return d.get(); }
	inline bool operator!() const { return !d; }

	inline ScExplicitlySharedPointer<T>& operator=(T* data);
	inline ScExplicitlySharedPointer<T>& operator=(const ScExplicitlySharedPointer<T>& o);
	inline ScExplicitlySharedPointer<T>& operator=(ScExplicitlySharedPointer<T>&& o) { d = o.d; o.data = nullptr; }

	inline operator bool() const { return d != nullptr; }

private:
	std::shared_ptr<T> d;
};

template <typename T>
inline void ScExplicitlySharedPointer<T>::detach()
{
	if (d && 1 != d.use_count())
		d = std::shared_ptr<T>(new T(*d));
}

template <typename T>
inline ScExplicitlySharedPointer<T>& ScExplicitlySharedPointer<T>::operator=(T* data)
{
	if (!d || d.get() != data)
		d = std::shared_ptr<T>(data);
	return *this;
}

template <typename T>
inline ScExplicitlySharedPointer<T>& ScExplicitlySharedPointer<T>::operator=(const ScExplicitlySharedPointer<T>& o)
{
	if (this != &o)
		d = o.d;
	return *this;
}

#endif // SCEXPLICITLYSHAREDPOINTER_H
