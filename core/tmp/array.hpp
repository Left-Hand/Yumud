#pragma once

#include <cstdint>


namespace ymd::tmp{

template<typename T>
static constexpr bool is_cstyle_array_v = false;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<T[N]> = true;

template<typename T, std::size_t N>
static constexpr bool is_cstyle_array_v<const T[N]> = true;



template<typename T>
static constexpr bool is_cstyle_char_array_v = false;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<char[N]> = true;

template<std::size_t N>
static constexpr bool is_cstyle_char_array_v<const char[N]> = true;

template<typename T>
struct _cstyle_array_size;

template<typename T, std::size_t N>
struct _cstyle_array_size<T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T, std::size_t N>
struct _cstyle_array_size<const T[N]> : std::integral_constant<std::size_t, N> {};

template<typename T>
static constexpr std::size_t cstyle_array_size_v = 
    _cstyle_array_size<T>::value;



static_assert(is_cstyle_char_array_v<char[8]> == true);
static_assert(is_cstyle_char_array_v<const char[8]> == true);
static_assert(is_cstyle_char_array_v<uint8_t[8]> == false);



}