#pragma once

#include "core/utils/stdrange.hpp"
#include <span>
#include <cstdint>
#include <bit>


namespace ymd{

template<std::ranges::range Range, typename T>
static constexpr bool is_range_value_convertible_v = 
    (std::is_convertible_v<std::ranges::range_value_t<Range>, T>);


using HashCode = uint32_t;

static constexpr HashCode HASHDJB_SEED = 5381;
namespace hashfunc{
    template <std::ranges::range Range>
    requires (is_range_value_convertible_v<Range, uint8_t> || is_range_value_convertible_v<Range, char>)
    __inline static constexpr uint32_t hash_uiml32(Range && range, uint32_t h = 0){
        const size_t str_length = std::size(range);
        size_t aligned_len = str_length / sizeof(uint32_t);
        for(size_t i = 0; i < aligned_len; ++i)  
            h = (h << 5) - h + (reinterpret_cast<const uint32_t*>(range)[i]); 
        for(size_t i = aligned_len << 2; i < str_length; ++i)
            h = (h << 5) - h + uint8_t(range[i]); 
        return h; 
    }

    template <std::ranges::range Range>
    requires (is_range_value_convertible_v<Range, uint8_t> || is_range_value_convertible_v<Range, char>)
    __inline static constexpr uint32_t hash_fnv1a(Range && range, uint32_t prev = 2166136261U){
        static constexpr uint32_t PRIME = 16777619U;
        const size_t len = std::size(range);
        for (size_t i = 0; i < len; i++) {
            prev ^= static_cast<uint32_t>(range[i]);
            prev *= PRIME;
        }
        return prev;
    }

    template <std::ranges::range Range>
    requires (is_range_value_convertible_v<Range, uint8_t> || is_range_value_convertible_v<Range, char>)
    __inline static constexpr HashCode hash_djb(Range && range, HashCode prev = HASHDJB_SEED) {
        for (const auto& byte : range) {
            prev = (prev * 33) ^ static_cast<uint8_t>(byte);
        }
        return prev;
    }

};




struct HashDjbBuilder final{
public:
    HashCode val_;
    explicit constexpr HashDjbBuilder(const HashCode val = HASHDJB_SEED):
        val_(val){;}

    static constexpr HashDjbBuilder from_default(){
        return HashDjbBuilder(HASHDJB_SEED);
    }

    constexpr HashCode finalize() const noexcept {return val_;} 

    template <typename T>
    constexpr HashDjbBuilder & operator << (T && obj){
        val_ =  hashfunc::hash_djb(obj, val_);
        return *this;
    }


    template<std::integral T>
    constexpr HashDjbBuilder & operator << (const T i){
        constexpr size_t N = sizeof(T);
        *this << std::bit_cast<std::array<uint8_t, N>>(i);  
        return *this;
    }


};

using DefaultHashBuilder = HashDjbBuilder;

template <typename HashBuilder = DefaultHashBuilder, typename ... Args>
__inline static constexpr HashCode hash(Args &&... args) {
    auto hasher = HashBuilder::from_default();
    return (hasher << ... << std::forward<Args>(args)).finalize();
}

__inline constexpr uint32_t operator ""_ha(char const* p, const size_t lenth)  {
    return hash<DefaultHashBuilder>(std::span<const char>{p, lenth});
}

}