#pragma once

#include "core/tmp/bits/width.hpp"
#include "core/tmp/container/static_string.hpp"
#include <string_view>
#include <optional>

// https://taylorconor.com/blog/enum-reflection/

namespace ymd::tmp{
namespace details{



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
    //             << os.field("scoped")(info.scoped) << os.splitter()
    //             << os.field("enum_name")(info.enum_name) << os.splitter()
    //             << os.field("value_fullname")(info.value_fullname) << os.splitter()
    //             << os.field("value_name")(info.value_name)

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
        constexpr auto str= tmp::static_string<value_name_view[indices]...>();
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

}
template <typename E, E V> 
constexpr std::string_view enum_item_name_v = tmp::details::_enum_item_name<E, V>();

template <typename E, E V> 
constexpr bool enum_is_valid_v = tmp::details::_enum_is_valid<E, V>();

template <typename E>
constexpr size_t enum_count_v = tmp::details::_enum_internal_element_count<E>(
    std::make_integer_sequence<size_t, 100>());

namespace details{
template<
    typename E, 
    size_t NUM_RANK,
    typename InnerInt = std::underlying_type_t<E>,
    InnerInt I
>
static consteval E _enum_find_item(){
    static_assert(sizeof(InnerInt) == 1);
    // constexpr InnerInt ENUM_INT_MIN = std::numeric_limits<InnerInt>::min();
    // constexpr InnerInt ENUM_INT_MAX = std::numeric_limits<InnerInt>::max();
    // static_assert(ENUM_INT_MAX - ENUM_INT_MIN <= 256);
    constexpr bool is_valid = tmp::enum_is_valid_v<E, static_cast<E>(I)>;
    if constexpr(is_valid){
        if constexpr (NUM_RANK == 0) return static_cast<E>(I);
        else return _enum_find_item<E, NUM_RANK - 1, InnerInt, I + 1>();
    }else{
        return _enum_find_item<E, NUM_RANK, InnerInt, I + 1>();
    }
}



template<typename E, size_t NUM_RANK>
static constexpr E _enum_item_v(){
    using InnerInt = std::underlying_type_t<E>;
    constexpr InnerInt ENUM_INT_MIN = std::numeric_limits<InnerInt>::min();
    return _enum_find_item<E, NUM_RANK, InnerInt, ENUM_INT_MIN>();
}
}

template<typename E, size_t NUM_RANK>
static constexpr E enum_item_v = details::_enum_item_v<E, NUM_RANK>();
template<typename Enum>
struct enum_dump{
    static constexpr size_t COUNT = tmp::enum_count_v<Enum>;
    using InnerInt = std::underlying_type_t<Enum>;

    using InnerIntStorage = std::array<InnerInt, COUNT>;
    using StringsStorage = std::array<std::string_view, COUNT>;

    static constexpr std::optional<std::string_view> enum_to_str(const Enum e_val){
        const auto rank = enum_to_rank(e_val);
        if(rank.has_value()) return strings_storage[rank.value()];
        else return std::nullopt;
    }

    static constexpr std::optional<Enum> str_to_enum(const std::string_view str){
        for(size_t i = 0; i < COUNT; ++i){
            if(str == strings_storage[i]) return static_cast<Enum>(inner_int_storage[i]);
        }
        return std::nullopt;
    }

    static constexpr std::optional<size_t> enum_to_rank(const Enum e_val){
        auto it = std::lower_bound(inner_int_storage.begin(), inner_int_storage.end(), static_cast<InnerInt>(e_val));
        if (it != inner_int_storage.end() && *it == static_cast<InnerInt>(e_val)) {
            return static_cast<int>(it - inner_int_storage.begin());
        }
        return std::nullopt;
    }
private:
    template<size_t... NUM_RANK>
    static constexpr auto make_inner_int_storage(std::index_sequence<NUM_RANK...>) {
        return InnerIntStorage{static_cast<InnerInt>(tmp::enum_item_v<Enum, NUM_RANK>)...};
    };
    
    
    template<size_t... NUM_RANK>
    static constexpr auto make_strings_storage(std::index_sequence<NUM_RANK...>) {
        return StringsStorage{tmp::enum_item_name_v<Enum, tmp::enum_item_v<Enum, NUM_RANK>>...};
    };

    static constexpr InnerIntStorage inner_int_storage = make_inner_int_storage(std::make_index_sequence<COUNT>{});
    static constexpr StringsStorage strings_storage = make_strings_storage(std::make_index_sequence<COUNT>{});
};
}

namespace ymd{
template<typename T>
struct DeriveDebugDispatcher;

template<typename E>
requires std::is_enum_v<E>
struct DeriveDebugDispatcher<E> {
    static void call(OutputStream& os, const E e_val) {
        const auto str_opt = tmp::enum_dump<E>::enum_to_str(e_val);
        if (str_opt.has_value()) {
            os << str_opt.value();
        } else {
            os << static_cast<std::underlying_type_t<E>>(e_val);
        }
    }
};
}
