#ifndef SCJSONDOCUMENT_H
#define SCJSONDOCUMENT_H

#include "scutils/serialization/scjsonvalue.h"

struct ScJsonParseError
{
	enum ParseError
	{
		NoError,
		UnterminatedArray,
		UnterminatedObject,
		UnterminatedString,
		IllegalValue,
		GarbageAtEnd
	};

	int		   offset;
	ParseError error;
};

class SC_API_EXPORT ScJsonDocument
{
public:
	enum JsonFormat
	{
		Indented,
		Compact
	};

	ScJsonDocument();
	explicit ScJsonDocument(const ScJsonArray& array);
	explicit ScJsonDocument(const ScJsonObject& object);
	ScJsonDocument(const ScJsonDocument& other);
	ScJsonDocument(ScJsonDocument&& other) noexcept : d(other.d) { other.d = nullptr; }
	~ScJsonDocument();

	bool isNull() const;
	bool isEmpty() const;
	bool isValid() const;
	bool isArray() const;
	bool isObject() const;

	ScJsonArray array() const;
	ScJsonObject object() const;

	void setArray(const ScJsonArray& array);
	void setObject(const ScJsonObject& object);

	ScString toJson(JsonFormat format = Compact) const;

	ScJsonDocument& operator =(const ScJsonDocument& other);
	inline ScJsonDocument& operator =(ScJsonDocument&& other) noexcept { std::swap(d, other.d); return *this; }

	static ScJsonDocument fromJson(const ScString& value, ScJsonParseError* error = nullptr);

private:
	ScJsonData* d;
};

#endif // SCJSONDOCUMENT_H   