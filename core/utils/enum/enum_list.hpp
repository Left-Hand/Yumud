#pragma once

#include <atomic>
#include <algorithm>
#include <array>
#include "core/tmp/bits/width.hpp"

namespace ymd{

template<typename E, E ... Elements> 
struct EnumList final{

public:
    using Enum = E;
    static constexpr size_t N = sizeof...(Elements);
private:
    static constexpr std::array<Enum, N> VALUES = std::to_array({Elements...});

    template<size_t ... Is>
    static consteval size_t enum_to_rank_tmp_impl(const E e, std::index_sequence<Is...>) {
        size_t result = N; // 初始化为无效值
        // 使用折叠表达式查找匹配的枚举值
        ((VALUES[Is] == e ? (result = Is, false) : false), ...);
        return result;
    }

public:

    static consteval size_t size(){
        return N;
    }

    static constexpr E rank_to_enum(size_t rank) {
        return VALUES[rank];
    }

    template<E e>
    static constexpr size_t enum_to_rank_v = enum_to_rank_tmp_impl(e, std::make_index_sequence<N>{});

    template<size_t rank>
    static constexpr E rank_to_enum_v = rank_to_enum(rank);

    static constexpr size_t enum_to_rank(const E e){
        return std::distance(VALUES.begin(),
            std::find(VALUES.begin(), VALUES.end(), e));
    }
};


}