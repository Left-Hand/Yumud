#pragma once

#include <optional>

namespace ymd::magic::details{
template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};

    constexpr explicit operator std::string_view()const{
        return std::string_view(__static_string::str, sizeof...(args));}
};


#if 0


// 用来存储枚举反射信息的结构体
// 注意名字都使用 string_view 存储，以避免动态内存分配
struct ReflectionEnumInfo
{      
    bool scoped; // 是否 scoped enum
    std::string_view enum_name;
    std::string_view value_fullname; 
    std::string_view value_name; // 类型名、值名、值全名

    // 构造时，从母串中按指定位置 得到各子串
    // info : 母串，即 __PRETTY_FUNCTION__ 得到的函数名
    // e1:等号1位置; s:分号位置; e2:等号2位置; colon:分号位置; end:]位置
    constexpr explicit ReflectionEnumInfo(char const* info
        , std::size_t e1, std::size_t s, std::size_t e2
        , std::size_t colon, std::size_t end)
        : scoped(colon != 0), enum_name (info + e1 + 2, s - e1 -2)
        , value_fullname (info + e2 + 2, end - e2 - 2)
        , value_name((scoped)? std::string_view(info+colon+1, end-colon-1) 
            : value_fullname)
    {;}

    // friend OutputStream & operator<<(OutputStream & os, const ReflectionEnumInfo & info){ 
    //     const auto guard = os.create_guard();
    //     os.no_scoped();
    //     return os << os.scoped("hi")(
    //             os
    //             << os.field("scoped")(os << info.scoped) << os.splitter()
    //             << os.field("enum_name")(os << info.enum_name) << os.splitter()
    //             << os.field("value_fullname")(os << info.value_fullname) << os.splitter()
    //             << os.field("value_name")(os << info.value_name)

    //         )
    //     ;
    // }
};

// 说了半天的 模板函数，带 constexpr
template <typename E, E V> 
consteval ReflectionEnumInfo dump_enum()  {
    char const* info = __PRETTY_FUNCTION__;
            
    // 找各个符号位置
    std::size_t l = strlen(info);
    std::size_t e1 = 0, s = 0, e2 = 0, colon = 0, end = 0;
    
    for (std::size_t i=0; i<l && !end; ++i)
    {
        switch(info[i])
        {
            case '=' : (!e1) ? e1 = i : e2 = i; break;            
            case ';' : s = i; break;                
            case ':' : colon = i; break;
            case ']' : end = i; break;
        }
    }
        
    return ReflectionEnumInfo{info, e1, s, e2, colon, end}; 
}

#else

// 用来存储枚举反射信息的结构体
// 注意名字都使用 string_view 存储，以避免动态内存分配
struct ReflectionEnumElementInfoV2
{      
    bool scoped; // 是否 scoped enum
    bool is_valid;
    std::string_view value_name; // 类型名、值名、值全名
};

// 说了半天的 模板函数，带 constexpr
template <typename E, E V> 
consteval ReflectionEnumElementInfoV2 dump_enum()  {
    constexpr std::string_view info = __PRETTY_FUNCTION__;
            
    // 找各个符号位置
    constexpr std::size_t l = info.length();

    constexpr auto result = [&]{
        std::size_t e1 = 0, s = 0, e2 = 0, colon = 0, end = 0;
        for (std::size_t i=0; i<l && !end; ++i){
            switch(info[i]){
                case '=' : (!e1) ? e1 = i : e2 = i; break;            
                case ';' : s = i; break;                
                case ':' : colon = i; break;
                case ']' : end = i; break;
            }
        }

        struct Res{
            std::size_t e1, s, e2, colon, end;
        };

        return Res{e1, s, e2, colon, end};
    }();

    // constexpr size_t e1 = result.e1;
    // constexpr size_t s = result.s;
    constexpr size_t e2 = result.e2;
    constexpr size_t colon = result.colon;
    constexpr size_t end = result.end;

    constexpr bool scoped = (colon != 0);
    constexpr std::string_view fullname = info.substr(e2 + 2, end - e2 - 2);
    constexpr bool is_valid = fullname[0] != '(';
    constexpr std::string_view value_name_view = ((scoped)? info.substr(colon+1, end-colon-1)
            : fullname);

    constexpr auto indices=std::make_index_sequence<value_name_view.size()>();
    constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
    {
        constexpr auto str= magic::details::__static_string<value_name_view[indices]...>();
        return str;
    }(indices);
    return ReflectionEnumElementInfoV2{
        scoped, 
        is_valid,
        std::string_view(type_name)
    };
}
#endif

template <typename E, E V>
consteval std::string_view _enum_item_name() {
    return dump_enum<E, V>().value_name;
}


template <typename E, E V> 
constexpr bool _enum_is_valid() {
    return dump_enum<E, V>().is_valid;
}


template <typename E> 
consteval size_t _enum_count_valid() {
    return 0;
}

template <typename E, E A, E... B> 
consteval size_t _enum_count_valid() {
    bool is_valid = _enum_is_valid<E, A>();
    return _enum_count_valid<E, B...>() + (size_t)is_valid;
}

template <typename E, size_t... I> 
consteval size_t _enum_internal_element_count(std::integer_sequence<size_t, I...> unused) {
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