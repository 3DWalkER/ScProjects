#ifndef SCSTRING_H
#define SCSTRING_H

#include "scutils/text/scstringdata.h"

class SC_API_EXPORT ScString
{
public:
	ScString() = default;
	ScString(const char* unicode, size_t size = -1);
	ScString(const std::string& str);
	ScString(const ScString& other) noexcept : d(other.d) {}
	inline ScString(ScString&& other) noexcept : d(std::move(other.d)) {}
	~ScString() = default;

	inline size_t size() const { return d.size(); }
	inline size_t length() const { return size(); }
	inline size_t count() const { return size(); }
	inline size_t capacity() const { return d.capacity(); }

	void resize(size_t size);
	void resize(size_t size, char fillChar);
	void reserve(size_t size) { d.reserve(size); }
	void truncate(size_t pos) { if (pos < size()) resize(pos); }
	void clear() { resize(0); }

	inline bool isEmpty() const { return 0 == size(); }

	char* data() { return d.data(); }
	const char* data() const { return d.data(); }

	ScString& assign(const char* s, const size_t n);
	ScString& append(const char* s, const size_t n);
	ScString& append(const ScString& str, const size_t pos, const size_t n);
	ScString& append(const ScString& str) { return append(str.data(), str.size()); }

	int compare(const char* s, const size_t n, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;
	int compare(const ScString& str, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return compare(str.data(), str.size(), cs); }
	static int compare(const ScString& lhs, const ScString& rhs, Sc::CaseSensitivity cs = Sc::CaseSensitive) { return lhs.compare(rhs, cs); }

	size_t indexOf(const char* s, size_t nsize, size_t pos, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;
	size_t indexOf(const ScString& lhs, size_t pos = 0, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return indexOf(lhs.data(), lhs.size(), pos, cs); }
	size_t indexOf(char c, size_t pos = 0, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return indexOf(&c, 1, pos, cs); }
	size_t indexOf(const char* s, size_t pos = 0, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return indexOf(s, SC_DETAIL::strlen_s(s), pos, cs); }

	size_t lastIndexOf(const char* s, size_t nsize, size_t pos, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;
	size_t lastIndexOf(const ScString& lhs, size_t pos = -1, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return lastIndexOf(lhs.data(), lhs.size(), pos, cs); }
	size_t lastIndexOf(const char* s, size_t pos = -1, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return lastIndexOf(s, SC_DETAIL::strlen_s(s), pos, cs); }
	size_t lastIndexOf(char c, size_t pos = -1, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return lastIndexOf(&c, 1, pos, cs); }

	bool contains(char ch, Sc::CaseSensitivity cs = Sc::CaseSensitive) { return npos != indexOf(ch, 0, cs); }

	bool startsWith(const char* s, size_t nsize, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;
	bool startsWith(const ScString& s, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return startsWith(s.data(), s.size(), cs); }
	bool startsWith(const char* s, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return startsWith(s, SC_DETAIL::strlen_s(s), cs); }
	bool startsWith(char c, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;

	bool endsWith(const char* s, size_t nsize, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;
	bool endsWith(const ScString& s, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return endsWith(s.data(), s.size(), cs); }
	bool endsWith(const char* s, Sc::CaseSensitivity cs = Sc::CaseSensitive) const { return endsWith(s, SC_DETAIL::strlen_s(s), cs); }
	bool endsWith(char c, Sc::CaseSensitivity cs = Sc::CaseSensitive) const;

	ScString mid(size_t pos = 0, size_t nsize = npos);

	typedef char* iterator;
	typedef const char* const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	iterator begin() { return d.mutableData(); }
	const_iterator begin() const { return d.data(); }
	const_iterator cbegin() const { return begin(); }
	iterator end() { return begin() + d.size(); }
	const_iterator end() const { return begin() + d.size(); }
	const_iterator cend() const { return end(); }
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return rbegin(); }
	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	const_reverse_iterator crend() const { return rend(); }

	ScString& remove(size_t pos = 0, size_t nsize = npos);
	iterator remove(const_iterator it);
	iterator remove(const_iterator first, const_iterator last);
	void chop(size_t n);

	const char at(size_t i) const { SC_ASSERT(i < size()); return data()[i]; }

	ScString& operator=(const ScString& other);
	ScString& operator=(ScString&& goner) noexcept;
	ScString& operator+=(const ScString& other) { return append(other); }
	ScString& operator+=(const char* s) { return append(s); }

	int toInt(bool* ok = nullptr, int base = 10);

	ScString trimmed() const&;
	ScString trimmed() const&&;

	static constexpr auto npos{ ScStringData::npos };

private:
	ScStringData d;
};

inline ScString ScString::mid(size_t pos, size_t nsize)
{
	SC_ASSERT(pos <= size());
	return ScString(data() + pos, scMin(size() - pos, nsize));
}

inline ScString::iterator ScString::remove(const_iterator it)
{
	const size_t pos(it - begin());
	SC_ASSERT(pos <= size());
	remove(pos, 1);
	return begin() + pos;
}

inline ScString::iterator ScString::remove(const_iterator first, const_iterator last)
{
	const size_t pos(first - begin());
	remove(pos, last - first);
	return begin() + pos;
}

inline void ScString::chop(size_t n)
{
	const auto size = length();
	resize(n > size ? 0 : size - n);
}

inline ScString& ScString::append(const ScString& str, const size_t pos, size_t n)
{
	const size_t max = str.size() - pos;
	n = scBound(size_t(0), n, max);
	return append(str.data() + pos, n);
}

inline ScString operator+(const ScString& lhs, const ScString& rhs)
{
	ScString result;
	result.reserve(lhs.size() + rhs.size());
	result.append(lhs).append(rhs);
	return result;
}

inline ScString operator+(ScString&& lhs, const ScString& rhs)
{
	return std::move(lhs.append(rhs));
}

inline ScString& ScString::operator=(const ScString& lhs)
{
	if (SC_UNLIKELY(&lhs == this))
		return *this;
	return assign(lhs.data(), lhs.size());
}

inline ScString& ScString::operator=(ScString&& goner) noexcept
{
	if (SC_UNLIKELY(&goner == this))
		return *this;

	this->~ScString();
	new (&d) ScStringData(std::move(goner.d));
	return *this;
}

inline ScString operator+(const ScString& lhs, ScString&& rhs)
{
	if (rhs.capacity() >= lhs.size() + rhs.size())
	{

	}
	const ScString& rhsC = rhs;
	return lhs + rhsC;
}

inline ScString operator+(ScString&& lhs, ScString&& rhs)
{
	return std::move(lhs.append(rhs));
}

inline bool operator==(const ScString& lhs, const ScString& rhs) {
	return lhs.size() == rhs.size() && 0 == lhs.compare(rhs);
}

inline ScString ScString::trimmed() const& {
	return ScString(*this).trimmed();
}

namespace std {
	template <>
	struct hash<ScString> {
		size_t operator()(const ScString& str) const noexcept {
			return _Hash_array_representation(str.data(), str.size());
		}
	};
}

#endif // SCSTRING_H