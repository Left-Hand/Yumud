#pragma once

#include <optional>

namespace ymd::magic::details{


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

template<size_t N, typename ... Args>
struct _tuple_element_t{};

template<size_t N, typename ... Args>
struct _tuple_element_t<N, std::tuple<Args...>>{
    using type = typename _element_t<N, Args...>::type;
};

template<size_t N, typename ... Args>
struct _total_bytes_of_args_v{
    static constexpr size_t value = _total_bytes_of_args_v<N-1, Args...>::value + sizeof(typename _element_t<N, Args...>::type);
};

template<typename ... Args>
struct _total_bytes_of_args_v<0, Args ...>{
    static constexpr size_t value = sizeof(typename _element_t<0, Args...>::type);
};

template<typename T>
struct _packed_tuple_total_bytes_v{};

template<typename ... Args>
struct _packed_tuple_total_bytes_v<std::tuple<Args...>>{
    static constexpr size_t value = _total_bytes_of_args_v<sizeof...(Args) - 1, Args...>::value;
};

template <size_t N, typename Tup>
struct _tuple_element_offset_v{
    static constexpr size_t value = 
        _tuple_element_offset_v<N - 1, Tup>::value + sizeof(std::tuple_element_t<N - 1, Tup>);
};


template<typename Tup>
struct _tuple_element_offset_v<0, Tup>{
    static constexpr size_t value = 0;
};


template<typename T>
struct _is_member_variable{
    static constexpr bool value = false;
};

template<typename T, typename U>
struct _is_member_variable<U T::*>{
    static constexpr bool value = true;
};
}

namespace ymd::magic{

// 只有在特化时才能够确定值 可用于constexpr排除分支下的static_assert
template <typename T>
static constexpr bool false_v = false;

template <typename T>
static constexpr bool true_v = true;


// template<typename T>
// static constexpr bool is_member_variable_v = details::_is_member_variable<decltype(&T::)>::value;

}