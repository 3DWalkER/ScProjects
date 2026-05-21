#ifndef SCJSONARRAY_H
#define SCJSONARRAY_H

#include "scutils/serialization/scjsonvalue.h"

class SC_API_EXPORT ScJsonArray
{
public:
	ScJsonArray();
	ScJsonArray(const ScJsonArray& other);
	ScJsonArray(ScJsonArray&& other) noexcept : d(other.d) { other.d = nullptr; }
	~ScJsonArray();

	int size() const;
	bool isEmpty() const { return !size(); }

	ScJsonValue at(int i) const;
	ScJsonValue first() const;
	ScJsonValue last() const;

	void prepend(const ScJsonValue& value);
	void append(const ScJsonValue& value);
	void insert(int i, const ScJsonValue& value);
	void replace(int i, const ScJsonValue& value);

	void removeAt(int i);
	void removeFirst() { removeAt(0); }
	void removeLast() { removeAt(size() - 1); }
	ScJsonValue takeAt(int i);

	ScJsonArray& operator=(const ScJsonArray& other);
	ScJsonArray& operator=(ScJsonArray&& other) noexcept { std::swap(d, other.d); return *this; }

private:
	ScJsonData* d;

	friend class ScJson;
	friend class ScJsonDocument;
	friend class ScJsonValueRef;
};

#endif // SCJSONARRAY_H