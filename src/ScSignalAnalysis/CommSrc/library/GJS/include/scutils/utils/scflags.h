#ifndef SCFLAGS_H
#define SCFLAGS_H

#include <condition_variable>

class ScFlag
{
	int i;
public:
	constexpr inline ScFlag(int value) noexcept : i(value) { }
	constexpr inline operator int() const noexcept { return i; }
};

template <typename Enum>
class ScFlags
{
	static_assert(sizeof(Enum) <= sizeof(int), "ScFlags uses an int as storage, so an enum with underlying long long will overflow.");
	static_assert(std::is_enum<Enum>::value, "ScFlags is only usable on enumeration types.");

	struct Private;
    typedef int Private::*Zero;

public:
#ifdef _MSC_VER
	typedef int Int;
#else
	typedef typename std::conditional<std::is_unsigned<typename std::underlying_type<Enum>::type>::value, unsigned int, signed int >::type Int;
#endif
	typedef Enum enum_type;

	constexpr inline ScFlags(Enum flags) noexcept : i(Int(flags)) { }
	constexpr inline ScFlags(Zero = nullptr) noexcept : i(0) { }
	constexpr inline ScFlags(ScFlag flag) noexcept : i(flag) { }

	constexpr inline operator Int() const noexcept { return i; }

	inline ScFlags &operator|=(ScFlags other) noexcept { i |= other.i; return *this; }
	inline ScFlags &operator|=(Enum other) noexcept { i |= Int(other); return *this; }
	inline ScFlags &operator^=(ScFlags other) noexcept { i ^= other.i; return *this; }
	inline ScFlags &operator^=(Enum other) noexcept { i ^= Int(other); return *this; }

	constexpr inline ScFlags operator|(ScFlags other) const noexcept { return ScFlags(ScFlag(i | other.i)); }
	constexpr inline ScFlags operator|(Enum other) const noexcept { return ScFlags(ScFlag(i | Int(other))); }
	constexpr inline ScFlags operator^(ScFlags other) const noexcept { return ScFlags(ScFlag(i ^ other.i)); }
	constexpr inline ScFlags operator^(Enum other) const noexcept { return ScFlags(ScFlag(i ^ Int(other))); }

	constexpr inline bool testFlag(Enum flag) const noexcept { return (i & Int(flag)) == Int(flag) && (Int(flag) != 0 || i == Int(flag)); }

private:
	Int i;
};

#define SC_DECLARE_FLAGS(Flags, Enum) \
    typedef ScFlags<Enum> Flags;

#define SC_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
    constexpr inline ScFlags<Flags::enum_type> operator|(Flags::enum_type f1, Flags::enum_type f2) noexcept \
	{ return ScFlags<Flags::enum_type>(f1) | f2; } \
    constexpr inline ScFlags<Flags::enum_type> operator|(Flags::enum_type f1, ScFlags<Flags::enum_type> f2) noexcept \
	{ return f2 | f1; }

#endif // SCFLAGS_H
