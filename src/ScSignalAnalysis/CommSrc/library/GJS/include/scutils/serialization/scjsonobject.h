#ifndef SCJSONOBJECT_H
#define SCJSONOBJECT_H

#include "scutils/serialization/scjsonvalue.h"

class SC_API_EXPORT ScJsonObject
{
public:
	ScJsonObject();
	ScJsonObject(const ScJsonObject& other);
	ScJsonObject(ScJsonObject&& other) noexcept : d(other.d) { other.d = nullptr; }
	~ScJsonObject();

	int size() const;
	bool isEmpty() const { return !size(); }

	void remove(const ScString& key);
	bool contains(const ScString& key) const;

	ScString keyAt(int i) const;
	ScJsonValue valueAt(int i) const;
	ScJsonValue value(const ScString& key) const;

	ScJsonValue operator[](const ScString& key) const { return value(key); }
	ScJsonValueRef operator[](const ScString& key);
	ScJsonObject& operator=(const ScJsonObject& other);
	ScJsonObject& operator=(ScJsonObject&& other) noexcept { std::swap(d, other.d); return *this; }

private:
	ScJsonData* d;

	friend class ScJson;
	friend class ScJsonDocument;
	friend class ScJsonValueRef;
};

#endif // SCJSONOBJECT_H