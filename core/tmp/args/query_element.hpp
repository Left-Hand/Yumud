#pragma once

#include <cstdint>
#include <tuple>

namespace ymd::tmp{


namespace details{
    template<size_t N, typename ... Args>
    struct _element_t{};

    template<typename First, typename ... Rest>
    struct _element_t<0, First, Rest...>{
        using type = First;
    };

    // template<typename First>
    // struct _element_t<0, First>{
    //     using type = First;
    // };

    template<size_t N, typename First, typename ... Rest>
    requires (N != 0)
    struct _element_t<N, First, Rest...>{
        static_assert(N < 1 + sizeof...(Rest), "tuple_element_bytes_v<N, Args...>: N >= sizeof...(Args)");

        using type = typename _element_t<N - 1, Rest...>::type;
    };
}
template<size_t N, typename ... Args>
using args_element_t = typename tmp::details::_element_t<N, Args...>::type;



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