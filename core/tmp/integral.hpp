#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <cstddef>

#include "core/tmp/bits/width.hpp"

namespace ymd::tmp{
namespace details{
template<typename U>
struct _extended_underlying;

template<>
struct _extended_underlying<uint8_t>{
    using type = uint16_t;
};

template<>
struct _extended_underlying<uint16_t>{
    using type = uint32_t;
};

template<>
struct _extended_underlying<uint32_t>{
    using type = uint64_t;
};

template<>
struct _extended_underlying<size_t>{
    using type = uint64_t;
};

template<>
struct _extended_underlying<uint64_t>{
    using type = uint64_t;
};

template<>
struct _extended_underlying<int8_t>{
    using type = int16_t;
};

template<>
struct _extended_underlying<int16_t>{
    using type = int32_t;
};

template<>
struct _extended_underlying<int32_t>{
    using type = int64_t;
};

template<>
struct _extended_underlying<int>{
    using type = int64_t;
};

template<>
struct _extended_underlying<int64_t>{
    using type = int64_t;
};

template<std::integral T1, std::integral T2>
struct _mul_underlying{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;

    static constexpr size_t max_size = 
        (sizeof(T1) > sizeof(T2) ? sizeof(T1) : sizeof(T2));

    using type = std::conditional_t<
        is_signed,
        bytes_to_sint_t<max_size>,
        bytes_to_uint_t<max_size>
    >;
};

template<std::integral T1, std::integral T2>
struct sum_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;

    static constexpr size_t max_size = 
        (sizeof(T1) > sizeof(T2) ? sizeof(T1) : sizeof(T2));

    using bigger_uint_type = bytes_to_uint_t<max_size>;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<bigger_uint_type>,
        bigger_uint_type
    >;
};


template<std::integral T1, std::integral T2>
struct _extended_mul_underlying{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;

    static constexpr size_t max_size = 
        (sizeof(T1) > sizeof(T2) ? sizeof(T1) : sizeof(T2));

    using bigger_uint_type = bytes_to_uint_t<max_size>;

    using extended_bigger_uint_type = typename _extended_underlying<bigger_uint_type>::type;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<extended_bigger_uint_type>,
        extended_bigger_uint_type
    >;
};
}

template<std::integral T1, std::integral T2>
using extended_mul_underlying_t = typename details::_extended_mul_underlying<T1, T2>::type;

template<typename T>
using extended_underlying_t = typename details::_extended_underlying<T>::type;

template<std::integral T1, std::integral T2>
using mul_underlying_t = typename details::_mul_underlying<T1, T2>::type;

template<std::integral T1, std::integral T2>
using sum_underlying_t = typename details::sum_underlying_type<T1, T2>::type;



static_assert(std::is_same_v<mul_underlying_t<int32_t, int32_t>, int32_t>);
static_assert(std::is_same_v<mul_underlying_t<uint32_t, int32_t>, int32_t>);
static_assert(std::is_same_v<mul_underlying_t<int16_t, uint8_t>, int16_t>);
static_assert(std::is_same_v<mul_underlying_t<long int, long int>, long int>);
static_assert(std::is_same_v<mul_underlying_t<int, int32_t>, int32_t>);
static_assert(std::is_same_v<mul_underlying_t<int, int>, int32_t>);


}