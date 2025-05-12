#pragma once

namespace ymd::magic::details{
template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};
    
    constexpr
    operator const char*()const{return __static_string::str;}
};




template <typename E, E V>
constexpr auto _enum_item_name() {
    constexpr std::string_view fully_name = __PRETTY_FUNCTION__;

    constexpr std::size_t begin=[&](){
        for(std::size_t i=fully_name.size() - 1;i>1;i--){
            const char chr = fully_name[i];
            if(chr ==' ' || chr == ')')
                return i + 1;
        }
    }();
    constexpr std::size_t end=[&](){
        for(std::size_t i=0;i<fully_name.size();i++)
            if(fully_name[i]==']')
                return i;
    }();
    
    constexpr auto type_name_view=fully_name.substr(begin,end-begin);
    constexpr auto indices=std::make_index_sequence<type_name_view.size()>();
    constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
    {
        constexpr auto str=__static_string<type_name_view[indices]...,'\0'>();
        return str;
    }(indices);
    return type_name;
}


template <typename E, E V> 
constexpr bool _enum_is_valid() {
    constexpr std::string_view fully_name = __PRETTY_FUNCTION__;

    constexpr std::size_t begin=[&](){
        for(std::size_t i=fully_name.size() - 1;i>1;i--){
            const char chr = fully_name[i];
            if(chr ==' ' || chr == ')')
                return i + 1;
        }
    }();
    constexpr std::size_t end=[&](){
        for(std::size_t i=0;i<fully_name.size();i++)
            if(fully_name[i]==']')
                return i;
    }();
    
    constexpr auto name=fully_name.substr(begin,end-begin);
    auto is_digit = [](char c) {
        return c >= '0' && c <= '9';
    };

    for(size_t i = 0; i < name.length(); i++){
        if(!is_digit(name[i])) return true;
    }
    return false;
}


template <typename E> 
consteval int _enum_count_valid() {
    return 0;
}

template <typename E, E A, E... B> 
consteval int _enum_count_valid() {
    bool is_valid = _enum_is_valid<E, A>();
    return _enum_count_valid<E, B...>() + (int)is_valid;
}

template <typename E, int... I> 
consteval int _enum_internal_element_count(std::integer_sequence<int, I...> unused) {
    return _enum_count_valid<E, (E)I...>();
}


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



}