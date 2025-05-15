#pragma once

#include <cstdint>
#include "typetraits_details.hpp"

namespace ymd::magic{


template<size_t N, typename ... Args>
using args_element_t = typename magic::details::_element_t<N, Args...>::type;

template<size_t N, typename ... Args>
static constexpr size_t element_bytes_v = sizeof(args_element_t<N, Args...>);

template<size_t N, typename Tup>
static constexpr size_t tuple_element_offset_v = magic::details::
    _tuple_element_offset_v<N, Tup>::value;


template<typename ... Args>
static constexpr size_t total_bytes_of_args_v = magic::details::
    _total_bytes_of_args_v<sizeof...(Args) - 1, Args...>::value;

template<typename Tup>
static constexpr size_t total_bytes_of_packed_tuple_v = magic::details::
    _packed_tuple_total_bytes_v<Tup>::value;

namespace details{
    template<typename T, typename... Ts>
    struct first_convertible_arg;

    template<typename T>
    struct first_convertible_arg<T> {
        using type = void;

        // static_assert(std::void_t<T>::value, "T must be a valid type");
    };

    template<typename T, typename U, typename... Ts>
    struct first_convertible_arg<T, U, Ts...> {
        using type = typename std::conditional<
            std::is_convertible<T, U>::value,
            U,
            typename first_convertible_arg<T, Ts...>::type
        >::type;
    };
}

//尝试将T逐一转换为Ts中的一个 如果都无法转换返回void
template<typename T, typename... Ts>
using first_convertible_arg_t = typename details::first_convertible_arg<T, Ts...>::type;

}