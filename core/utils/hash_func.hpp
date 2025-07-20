#pragma once

#include "core/platform.hpp"
#include "core/utils/stdrange.hpp"
#include <span>


namespace ymd{

template<std::ranges::range Range, typename T>
static constexpr bool is_range_value_convertible_v = 
    (std::is_convertible_v<std::ranges::range_value_t<Range>, T>);
using HashCode = uint32_t;

static constexpr HashCode HASHDJB_SEED = 5381;

enum class HashAlgo:uint8_t{
    Djb
};

namespace hashfunc{

    __inline static constexpr uint32_t hash_uiml32(char const*data, size_t length){
        uint32_t h = 0;  
        uint16_t strLength = length, alignedLen = strLength / sizeof(uint32_t);
        for(size_t i = 0; i < alignedLen; ++i)  
            h = (h << 5) - h + (reinterpret_cast<const uint32_t*>(data)[i]); 
        for(size_t i = alignedLen << 2; i < strLength; ++i)
            h = (h << 5) - h + uint8_t(data[i]); 
        return h; 
    }


    __inline static constexpr uint32_t hash_fnv1a(char const*data, size_t length){
        uint32_t prime = 16777619U;
        uint32_t ret = 2166136261U;
        for (size_t i = 0; i < length; i++) {
            ret ^= uint32_t(data[i]);
            ret *= prime;
        }
        return ret;
    }

    /**
     * @brief DJB2 哈希算法
     * @tparam Range 输入范围（必须满足 std::ranges::range 且元素可转 uint8_t）
     * @param range 输入数据范围
     * @param last 上一次的哈希值（默认 5381，DJB2 初始种子）
     * @return 计算后的新哈希值
     */
    template <std::ranges::range Range>
    requires (is_range_value_convertible_v<Range, uint8_t> || is_range_value_convertible_v<Range, char>)
    __inline static constexpr HashCode hash_djb(Range && range, HashCode last = HASHDJB_SEED) {
        for (const auto& byte : range) {
            last = (last * 33) ^ static_cast<uint8_t>(byte);
        }
        return last;
    }

    template<typename Iter>
    requires (is_next_based_iter_v<Iter>)
    __inline static constexpr HashCode hash_djb(Iter iter, HashCode last = HASHDJB_SEED){
        while(iter.has_next()){
            last = (last * 33) ^ std::bit_cast<uint8_t>(iter.next());
        }
        return last;
    }
};




template<HashAlgo S>
struct Hasher{

};

template<>
struct Hasher<HashAlgo::Djb>{
public:
    explicit constexpr Hasher(const HashCode code = HASHDJB_SEED):
        code_(code){;}

    constexpr HashCode code() const{return code_;} 

    template <typename T>
    constexpr Hasher & operator << (T && obj){
        code_ =  hashfunc::hash_djb(obj, code_);
        return *this;
    }


    template<std::integral T>
    constexpr Hasher & operator << (const T i){
        constexpr size_t N = sizeof(T);
        *this << std::bit_cast<std::array<uint8_t, N>>(i);
        return *this;
    }
private:
    HashCode code_;
};

template <HashAlgo S = HashAlgo::Djb, typename ... Args>
__inline static constexpr HashCode hash(Args &&... args) {
    auto hasher = Hasher<S>{};
    return (hasher << ... << std::forward<Args>(args)).code();
}

__inline constexpr uint32_t operator "" _ha(char const* p, const size_t size)  {
    return hash(std::span<const char>{p, size});
}

}