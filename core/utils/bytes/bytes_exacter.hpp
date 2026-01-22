#pragma once

#include <cstdint>
#include <span>
#include "core/utils/bits/bits_caster.hpp"
#include "core/polymorphism/reflect.hpp"

namespace ymd{

template<std::endian E, size_t Extents>
struct [[nodiscard]] BytesExacter {
    static_assert(Extents != std::dynamic_extent);
    constexpr explicit BytesExacter(std::span<const uint8_t, Extents> bytes) : 
        bytes_(bytes) {;}

    template<typename ... Ts>
    requires ((tmp::total_bytes_of_bits_ctorable_v<std::tuple<Ts...>> == Extents))
    constexpr void exact_to_elements(Ts&... ts) const {
        // 使用折叠表达式为每个参数调用绑定函数
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (exact_to_element<Indices, std::tuple<Ts...>>(ts), ...);
        }(std::make_index_sequence<sizeof...(Ts)> {});
    }

    template<typename T> 
    [[nodiscard]] constexpr T make_struct() const {
        using Tup = std::decay_t<decltype(reflect::to<std::tuple>(std::declval<T>()))>;
        T ret;
        using Is = std::make_index_sequence<std::tuple_size_v<Tup>>;
        auto construct_tuple = [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (exact_to_element<Indices, Tup>(reflect::get<Indices, T &>(ret)), ...);
        };
        construct_tuple(Is{});
        return ret;
    }
private:
    std::span<const uint8_t, Extents> bytes_;

    // 为单个元素绑定数据
    template<
        size_t I, 
        typename Tup, 
        typename T = std::tuple_element_t<I, Tup >
        >
    constexpr __always_inline void exact_to_element(T& element) const {
        using D = tmp::from_bits_t<T>;
        constexpr size_t WIDTH = sizeof(D);
        constexpr size_t OFFSET = tmp::offset_of_bits_ctorable_v<I, Tup>;

        static_assert(OFFSET + WIDTH <= Extents);
        
        // 从字节数据中构造元素并赋值给引用
        element = BytesCtorBits<E, WIDTH>(std::span<const uint8_t, WIDTH>(
            bytes_.data() + OFFSET, WIDTH
        ));
    }
};


template<std::endian E, size_t Extents>
BytesExacter(std::span<const uint8_t, Extents>) -> BytesExacter<E, Extents>;

template<std::endian E, size_t N>
static constexpr auto make_bytes_exacter(std::span<const uint8_t, N> bytes){
    return BytesExacter<E, N>(bytes);
}
}