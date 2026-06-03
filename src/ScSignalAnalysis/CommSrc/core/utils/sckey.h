#ifndef SCKEY_H
#define SCKEY_H

#include <type_traits>

/**
 * @brief ScUsrKey 任务用户索引键
 */
struct ScUsrKey
{
	usrid id;           /**< 用户ID */ 
	size_t channel;     /**< 通道号，主通道和子通道组合的结果 */ 
	bool operator==(const ScUsrKey& other) const {
		return id == other.id && channel == other.channel;
	}
};

struct ScUsrKeyHasher
{
	size_t operator()(const ScUsrKey& k) const {
		return std::hash<usrid>{}(k.id) ^ (std::hash<size_t>{}(k.channel) << 1);
	}
};


/**
 * @brief ScChannelKey 任务通道索引键，通过 channel 和 subChannel 组合
 */
typedef struct ScChannelKey
{
    int channel;        /**< 主通道号 */
    int subChannel;     /**< 子通道号 */

    bool operator==(const ScChannelKey& other) const {
        return channel == other.channel && subChannel == other.subChannel;
    }

    operator std::size_t() const {
        const uint32_t c = static_cast<uint32_t>(channel);
        const uint32_t s = static_cast<uint32_t>(subChannel);
        return (static_cast<uint64_t>(c) << 32) | static_cast<uint64_t>(s);
    }
} ScChannel;

struct ScChannelKeyHasher {
    std::size_t operator()(const ScChannelKey& k) const {
        return std::hash<int>()(k.channel) ^ (std::hash<int>()(k.subChannel) << 1);
    }
};

#endif // SCKEY_H
