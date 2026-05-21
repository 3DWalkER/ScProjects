#ifndef SCSTRINGLIST_H
#define SCSTRINGLIST_H

#include "scutils/text/scstring.h"
#include "scutils/utils/screfcount.h"
#include "scutils/container/sccontainerutils.h"
#include <vector>

struct ScStringListData
{
	typedef std::vector<ScString> Container;
	ScRefCount ref;
	Container list;
	static ScStringListData shared_null;
};

class SC_API_EXPORT ScStringList
{
public:
	ScStringList();
	explicit ScStringList(const ScString& s) { append(s); }
	ScStringList(const ScStringList &other);
	ScStringList(std::initializer_list<ScString> args);
	ScStringList(ScStringList&& goner) noexcept : d(std::move(goner.d)) { goner.d = &ScStringListData::shared_null; }
	template <typename InputIterator, Sc::IfIsInputIterator<InputIterator> = true>
	ScStringList(InputIterator first, InputIterator second);
	~ScStringList();

	size_t length() const { return d->list.size(); }
	size_t size() const { return length(); }
	bool isEmpty() const { return d->list.empty(); }

	void reserve(size_t size) { return d->list.reserve(size); }
	void detach();

	void append(const ScString& s) { *this += s; }
	void append(const ScStringList& list) { *this += list; }
	void prepend(const ScString& s);
	void prepend(const ScStringList& list);

	typedef ScStringListData::Container Container;
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;
	typedef Container::reverse_iterator reverse_iterator;
	typedef Container::const_reverse_iterator const_reverse_iterator;

	iterator begin() noexcept { detach(); return d->list.begin(); }
	const_iterator begin() const noexcept { return d->list.cbegin(); }
	const_iterator cbegin() const noexcept { return begin(); }
	iterator end() noexcept { detach(); return d->list.end(); }
	const_iterator end() const noexcept { return d->list.cend(); }
	const_iterator cend() const noexcept { return end(); }
	reverse_iterator rbegin() noexcept { detach(); return d->list.rbegin(); }
	const_reverse_iterator rbegin() const noexcept { return d->list.rbegin(); }
	const_reverse_iterator crbegin() const noexcept { return rbegin(); }
	reverse_iterator rend() noexcept { detach(); return d->list.rend(); }
	const_reverse_iterator rend() const noexcept { return d->list.rend(); }
	const_reverse_iterator crend() const noexcept { return rend(); }

	const ScString& at(size_t i) const;
	const ScString& operator[](size_t i) const { return at(i); }
	ScString& operator[](size_t i);

	ScStringList& operator=(const ScStringList& list);
	ScStringList& operator=(ScStringList&& goner) noexcept;
	ScStringList& operator+=(const ScString& s);
	ScStringList& operator+=(const ScStringList& list);
	ScStringList& operator<<(const ScString& s) { *this += s; return *this; }
	ScStringList& operator<<(const ScStringList& list) { *this += list; return *this; }

private:
	ScStringListData* d;
};
template<typename InputIterator, Sc::IfIsInputIterator<InputIterator>>
inline ScStringList::ScStringList(InputIterator first, InputIterator second)
	: d(new ScStringListData())
{
	d->list = std::move(Container(first, second));
	d->ref.setSharable(true);
}

inline const ScString& ScStringList::at(size_t i) const
{
	SC_ASSERT_X(i < size(), "ScStringList::at", "index out of range");
	return d->list.at(i);
}

inline ScString& ScStringList::operator[](size_t i)
{
	SC_ASSERT_X(i < size(), "ScStringList::at", "index out of range");
	detach();
	return d->list.at(i);
}

inline ScStringList& ScStringList::operator=(const ScStringList& list)
{
	if (SC_LIKELY(list.d != d))
	{
		ScStringList nascent(list);
		std::swap(nascent.d, d);
	}
	return *this;
}

inline ScStringList& ScStringList::operator=(ScStringList&& goner) noexcept
{
	if (SC_LIKELY(&goner != this))
	{
		ScStringList nascent(std::move(goner));
		std::swap(nascent.d, d);
	}
	return *this;
}

#endif // SCSTRINGLIST_H
