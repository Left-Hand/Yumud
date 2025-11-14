#pragma once

#include <cstdint>
#include <span>
#include "core/utils/bits_caster.hpp"
#include "core/polymorphism/reflect.hpp"

namespace ymd{

template<Endian::Kind E, size_t Extents>
struct [[nodiscard]] BytesCtorBitsExacter {
    static_assert(Extents != std::dynamic_extent);
    constexpr explicit BytesCtorBitsExacter(std::span<const uint8_t, Extents> bytes) : 
        bytes_(bytes) {;}

    template<typename ... Ts>
    requires ((total_bytes_of_bits_ctorable_v<std::tuple<Ts...>> == Extents))
    constexpr void exact_to_elements(Ts&... ts) const {
        // 使用折叠表达式为每个参数调用绑定函数
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            (exact_to_element<Indices, std::tuple<Ts...>>(ts), ...);
        }(std::make_index_sequence<sizeof...(Ts)> {});
    }

    // template<typename ... Ts>
    // requires (total_bytes_of_bits_ctorable_v<Ts...> == Extents)
    // [[nodiscard]] constexpr std::tuple<Ts...> deconstruct_to_tuple() const{
    //     using Tup = std::tuple<Ts...>;
    //     static constexpr size_t N = sizeof...(Ts);
    //     union{Tup ret;};

    //     using Is = std::make_index_sequence<std::tuple_size_v<Tup>>;
    //     auto construct_tuple = [this]<std::size_t... Indices>(std::index_sequence<Indices...>) {
    //         (deconstruct_element_from_total_bytes<Indices, Tup>(bytes_), ...);
    //     };
    //     construct_tuple(Is{});
    //     return ret;
    // }

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
        using D = from_bits_t<T>;
        constexpr size_t WIDTH = sizeof(D);
        constexpr size_t OFFSET = offset_of_bits_ctorable_v<I, Tup>;

        static_assert(OFFSET + WIDTH <= Extents);
        
        // 从字节数据中构造元素并赋值给引用
        element = BytesCtorBits<E, WIDTH>(std::span<const uint8_t, WIDTH>(
            bytes_.data() + OFFSET, WIDTH
        ));
    }
};


template<Endian::Kind E, size_t Extents>
BytesCtorBitsExacter(std::span<const uint8_t, Extents>) -> BytesCtorBitsExacter<E, Extents>;

template<Endian::Kind E, size_t N>
static constexpr auto make_bytes_ctor_bits_exacter(std::span<const uint8_t, N> bytes){
    return BytesCtorBitsExacter<E, N>(bytes);
}
}