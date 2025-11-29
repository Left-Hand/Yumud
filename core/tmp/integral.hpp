#pragma once

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <cstddef>

namespace ymd::tmp{

template<typename U>
struct extended_underlying_type;

template<>
struct extended_underlying_type<uint8_t>{
    using type = uint16_t;
};

template<>
struct extended_underlying_type<uint16_t>{
    using type = uint32_t;
};

template<>
struct extended_underlying_type<uint32_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<size_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<uint64_t>{
    using type = uint64_t;
};

template<>
struct extended_underlying_type<int8_t>{
    using type = int16_t;
};

template<>
struct extended_underlying_type<int16_t>{
    using type = int32_t;
};

template<>
struct extended_underlying_type<int32_t>{
    using type = int64_t;
};

template<>
struct extended_underlying_type<int>{
    using type = int64_t;
};

template<>
struct extended_underlying_type<int64_t>{
    using type = int64_t;
};

template<std::integral T1, std::integral T2>
struct mul_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<bigger_uint_type>,
        bigger_uint_type
    >;
};

template<std::integral T1, std::integral T2>
struct sum_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<bigger_uint_type>,
        bigger_uint_type
    >;
};


static_assert(std::is_same_v<typename mul_underlying_type<int32_t, int32_t>::type, int32_t>);
static_assert(std::is_same_v<typename mul_underlying_type<long int, long int>::type, long int>);

template<std::integral T1, std::integral T2>
struct extended_mul_underlying_type{
    static constexpr bool is_signed = std::is_signed_v<T1> || std::is_signed_v<T2>;
    using unsigned_t1_type = std::make_unsigned_t<T1>;
    using unsigned_t2_type = std::make_unsigned_t<T2>;

    using bigger_uint_type = std::conditional_t<
        (sizeof(unsigned_t1_type) > sizeof(unsigned_t2_type)),
        unsigned_t1_type, unsigned_t2_type
    >;

    using extended_bigger_uint_type = typename extended_underlying_type<bigger_uint_type>::type;

    using type = std::conditional_t<
        is_signed,
        std::make_signed_t<extended_bigger_uint_type>,
        extended_bigger_uint_type
    >;
};

template<std::integral T1, std::integral T2>
using extended_mul_underlying_t = typename extended_mul_underlying_type<T1, T2>::type;

template<typename T>
using extended_underlying_t = typename extended_underlying_type<T>::type;

template<std::integral T1, std::integral T2>
using mul_underlying_t = typename mul_underlying_type<T1, T2>::type;

template<std::integral T1, std::integral T2>
using sum_underlying_t = typename sum_underlying_type<T1, T2>::type;

}