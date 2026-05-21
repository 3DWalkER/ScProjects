#ifndef SCBIHASH_H
#define SCBIHASH_H

#include <unordered_map>
#include <functional>

template <class K, class V>
class ScBiHash
{
public:
	/**
	 * @brief ScBiHash 默认构造函数
	 */
	ScBiHash() {}

	/**
	 * @brief ScBiHash 使用初始化列表构造
	 */
	ScBiHash(std::initializer_list<std::pair<K, V>> list)
	{
		for (const auto& pair : list)
			insert(pair.first, pair.second);
	}

	/**
	 * @brief ScBiHash 用std::unordered_map构造
	 */
	ScBiHash(const std::unordered_map<K, V>& other) { unite(other); }

	/**
	 * @brief ScBiHash 拷贝构造函数
	 */
	ScBiHash(const ScBiHash<K, V>& other) : hash(other.hash), inverted(other.inverted) {}

	/**
	 * @brief ScBiHash 移动构造函数
	 */
	ScBiHash(ScBiHash<K, V>&& other) : hash(std::move(other.hash)), inverted(std::move(other.inverted)) {}

	/**
	 * @brief containsKey 判断哈希表中是否包含指定的键
	 */
	inline bool containsKey(const K& key) const { return hash.end() != hash.find(key); }

	/**
	 * @brief containsKey 判断哈希表中是否包含指定的值
	 */
	inline bool containsValue(const V& val) const { return inverted.end() != inverted.find(val); }

	/**
	 * @brief key 根据值获取键
	 */
	inline K key(const V& val) const
	{
		auto it = inverted.find(val);
		return inverted.end() != it ? it->second : V();
	}

	/**
	 * @brief value 根据键获取值
	 */
	inline V value(const K& key) const
	{
		auto it = hash.find(key);
		return hash.end() != it ? it->second : V();
	}

	/**
	 * @brief takeKey 弹出并获取给定值对应键
	 */
	K takeKey(const V& val)
	{
		K k{ };
		auto it = inverted.find(val);
		if (inverted.end() != it)
		{
			k = it->second;
			inverted.erase(it);
			hash.erase(k);
		}

		return k;
	}

	/**
	 * @brief takeValue 弹出并获取给定键对应键值
	 */
	V takeValue(const K& key)
	{
		V v{ };
		auto it = hash.find(key);
		if (hash.end() != it)
		{
			v = it->second;
			hash.erase(it);
			inverted.erase(v);
		}

		return v;
	}

	/**
	 * @brief insert 在哈希表中插入新的键值对, 如果存在涉替换之前的
	 */
	inline void insert(const K& key, const V& val)
	{
		hash[key] = val;
		inverted[val] = key;
	}

	/**
	 * @brief removeKey 移除哈希表中指定的键
	 */
	inline int removeKey(const K& key)
	{
		if (!containsKey(key))
			return 0;

		inverted.erase(hash.at(key));
		hash.erase(key);
		return 1;
	}

	/**
	 * @brief removeKey 移除哈希表中指定的值
	 */
	inline int removeValue(const V& key)
	{
		if (!containsValue(key))
			return 0;

		hash.erase(inverted.at(key));
		inverted.erase(key);
		return 1;
	}

	/**
	 * @brief unite 将外部的std::unordered_map中所有元素添加到该哈希表中
	 */
	ScBiHash<K, V>& unite(const std::unordered_map<K, V>& other)
	{
		for (const auto& pair : other)
			insert(pair.first, pair.second);
		return *this;
	}

	/**
	 * @brief iterator <K, V>哈希表迭代器
	 */
	inline void iterator(std::function<void(const K& key, const V& value)> func) const {
		for (auto it = hash.cbegin(); it != hash.cend(); ++it)
			func(it->first, it->second);
	}

	/**
	 * @brief cbegin <K, V>哈希表迭代器
	 */
	inline typename std::unordered_map<K, V>::const_iterator cbegin() const { return hash.cbegin(); }

	/**
	 * @brief count 统计哈希表中条目数
	 */
	inline size_t size() const { return hash.size(); }

	/**
	 * @brief isEmpty 判断该哈希表是否为空
	 */
	inline bool isEmpty() const { return hash.empty(); }

	/**
	 * @brief clear 清除哈希表中所有条目
	 */
	inline void clear() { hash.clear(); inverted.clear(); }

private:
	/**
	 * @brief hash 从左到右映射的哈希表
	 */
	std::unordered_map<K, V> hash;

	/**
	 * @brief inverted 从右到左映射的哈希表
	 */
	std::unordered_map<V, K> inverted;
};

#endif // SCBIHASH_H
