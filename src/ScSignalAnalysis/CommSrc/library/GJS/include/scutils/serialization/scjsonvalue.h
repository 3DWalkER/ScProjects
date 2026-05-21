#ifndef SCJSONVALUE
#define SCJSONVALUE

#include "scutils/text/scstring.h"

class ScJsonData;
class ScJsonObject;
class ScJsonArray;
class ScJsonValuePrivate;
class ScJsonValueRefPrivate;

class SC_API_EXPORT ScJsonValue
{
	SC_DECLARE_PRIVATE(ScJsonValue)
public:
	enum Type
	{
		Null,
		Bool,
		Number,
		String,
		Array,
		Object,
		Invalid,
	};

	ScJsonValue(Type type = Null);
	ScJsonValue(bool value);
	ScJsonValue(int value);
	ScJsonValue(scuint64 value);
	ScJsonValue(scint64 value);
	ScJsonValue(double value);
	ScJsonValue(const ScString& value);
	ScJsonValue(const char* value);
	ScJsonValue(const ScJsonObject& value);
	ScJsonValue(const ScJsonArray& value);
	ScJsonValue(const ScJsonValue& other);
	ScJsonValue(ScJsonValue&& goner) noexcept;
	~ScJsonValue();

	Type type() const;
	inline bool isNull() const { return type() == Null; }
	inline bool isBool() const { return type() == Bool; }
	inline bool isNumber() const { return type() == Number; }
	inline bool isString() const { return type() == String; }
	inline bool isArray() const { return type() == Array; }
	inline bool isObject() const { return type() == Object; }
	inline bool isInvalid() const { return type() == Invalid; }

	bool toBool(bool defaultValue = false) const;
	int toInt(int defaultValue = 0) const;
	double toDouble(double defaultValue = 0) const;
	ScString toString(const ScString& defaultValue = ScString()) const;
	ScJsonArray toArray() const;
	ScJsonArray toArray(const ScJsonArray& defaultValue) const;
	ScJsonObject toObject() const;
	ScJsonObject toObject(const ScJsonObject& defaultValue) const;

	ScJsonValue& operator=(const ScJsonValue& other);
	ScJsonValue& operator=(ScJsonValue&& goner) noexcept;

	bool operator==(const ScJsonValue& other) const;
	inline bool operator!=(const ScJsonValue& other) const { return !(*this == other); }

private:
	ScJsonValuePrivate* d_ptr;
};

inline ScJsonValue::ScJsonValue(ScJsonValue&& goner) noexcept
	: d_ptr(goner.d_ptr)
{
	goner.d_ptr = nullptr;
}

inline ScJsonValue& ScJsonValue::operator=(ScJsonValue&& goner) noexcept
{
	std::swap(d_ptr, goner.d_ptr);
	return *this;
}

class SC_API_EXPORT ScJsonValueRef
{
	SC_DECLARE_PRIVATE(ScJsonValueRef)
public:
	ScJsonValueRef(ScJsonArray* array, int index);
	ScJsonValueRef(ScJsonObject* object, const ScString& key);
	~ScJsonValueRef();

	ScJsonValue::Type type() const;
	inline bool isNull() const { return type() == ScJsonValue::Null; }
	inline bool isBool() const { return type() == ScJsonValue::Bool; }
	inline bool isNumber() const { return type() == ScJsonValue::Number; }
	inline bool isString() const { return type() == ScJsonValue::String; }
	inline bool isArray() const { return type() == ScJsonValue::Array; }
	inline bool isObject() const { return type() == ScJsonValue::Object; }
	inline bool isInvalid() const { return type() == ScJsonValue::Invalid; }

	bool toBool(bool defaultValue = false) const;
	int toInt(int defaultValue = 0) const;
	double toDouble(double defaultValue = 0) const;
	ScString toString(const ScString& defaultValue = ScString()) const;
	ScJsonArray toArray() const;
	ScJsonArray toArray(const ScJsonArray& defaultValue) const;
	ScJsonObject toObject() const;
	ScJsonObject toObject(const ScJsonObject& defaultValue) const;

	operator ScJsonValue() const;

	ScJsonValueRef& operator=(const ScJsonValue& val);
	bool operator==(const ScJsonValue& other) const;
	bool operator!=(const ScJsonValue& other) const { return !(*this == other); }

private:
	ScJsonValueRefPrivate* d_ptr;
};

#endif // SCJSONVALUE
