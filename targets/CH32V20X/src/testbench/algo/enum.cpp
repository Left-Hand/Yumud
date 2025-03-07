#include "../tb.h"

// #include 
// https://taylorconor.com/blog/enum-reflection/

template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};
    
    constexpr
    operator const char*()const{return __static_string::str;}
};

namespace details{
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

}

using namespace details;


namespace details{
    template<size_t N, typename ... Args>
    struct _element_t{};
    
    template<typename First, typename ... Rest>
    struct _element_t<0, First, Rest...>{
        using type = First;
    };

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
    struct _total_bytes_v{
        static constexpr size_t value = _total_bytes_v<N-1, Args...>::value + sizeof(typename _element_t<N, Args...>::type);
    };

    template<typename ... Args>
    struct _total_bytes_v<0, Args ...>{
        static constexpr size_t value = sizeof(typename _element_t<0, Args...>::type);
    };

    template<typename T>
    struct _tuple_total_bytes_v{};

    template<typename ... Args>
    struct _tuple_total_bytes_v<std::tuple<Args...>>{
        static constexpr size_t value = _total_bytes_v<sizeof...(Args) - 1, Args...>::value;
    };

}

using namespace details;



template <typename E, E V> 
constexpr const char * enum_item_name_v = _enum_item_name<E, V>();


template <typename E, E V> 
constexpr bool enum_is_valid_v = _enum_is_valid<E, V>();


template <typename E>
constexpr int enum_count_v = _enum_internal_element_count<E>(std::make_integer_sequence<int, 100>());


template<size_t N, typename ... Args>
using element_t = typename _element_t<N, Args...>::type;

template<size_t N, typename ... Args>
static constexpr size_t element_bytes_v = sizeof(element_t<N, Args...>);


template<size_t N, typename Tup>
using tuple_element_t = typename _tuple_element_t<N, Tup>::type;


template<typename ... Args>
static constexpr size_t total_bytes_v = _total_bytes_v<sizeof...(Args) - 1, Args...>::value;


template<typename Tup>
static constexpr size_t tuple_total_bytes_v = _tuple_total_bytes_v<Tup>::value;

template<typename T>
static constexpr bool is_functor_v = requires(T t){t();};

enum Fruit {
    BANANA = 5,
    APPLE = 12,
};

// template<typename T>
// void test(){
//     static_assert(false_v<T>, "ji", __FUNCTION__);
// }


void enum_main(){
    [[maybe_unused]] constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
    [[maybe_unused]] constexpr const char * _10 = enum_item_name_v<Fruit, Fruit(10)>;

    constexpr size_t count = enum_count_v<Fruit>;
    // constexpr const char * banana2 = enum_item_name_v2<Fruit::BANANA>;
    // using type = decltype(Fruit::BANANA);
    // constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
 
    // static_assert(!, "!enum_is_valid_v<Fruit, Fruit::10>()");
    // const auto f = __PRETTY_FUNCTION__;
    static_assert(enum_is_valid_v<Fruit, Fruit::BANANA>, "enum_is not _valid_v<Fruit, Fruit::BANANA>()");
    static_assert(!enum_is_valid_v<Fruit, Fruit(10)>, "enum_is_valid_v<Fruit, Fruit::10>()");
    // static_assert(!enum_is_valid_v<Fruit, Fruit>, "!enum_is_valid_v<Fruit, Fruit::10>()");

    // static constexpr size_t a = tuple_bytes_v<int, int, uint8_t>;
    {
        using type = std::tuple<int, uint16_t, uint8_t, iq_t<16>>;
        
        // static constexpr size_t a1 = tuple_element_bytes_v<1, type>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, type>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, type>;

        // static constexpr size_t a3 = element_bytes_v<2, int, uint16_t, uint8_t, iq_t<16>>;
        using t0 = element_t<0, int, uint16_t, uint8_t, iq_t<16>>;
        using t1 = element_t<1, int, uint16_t, uint8_t, iq_t<16>>;
        
        using t3 = tuple_element_t<3, std::tuple<int, uint16_t, uint8_t, iq_t<16>>>;
        // test<t2>();

        static_assert(std::is_same_v<t0, int>);
        static_assert(std::is_same_v<t1, uint16_t>);
        static_assert(std::is_same_v<t3, iq_t<16>>);

        // static_assert(tuple_total_bytes_v<type> == sizeof(type));
        // total_bytes_v<int, uint16_t, uint8_t, iq_t<16>>;
        // tuple_total_bytes_v<type>;
        // static constexpr size_t a0 = element_bytes_v<0, int, uint16_t, uint8_t, iq_t<16>>;
        // static constexpr size_t a1 = element_bytes_v<1, int, uint16_t, uint8_t, iq_t<16>>;

        // static constexpr size_t a = element_bytes_v<1, int, short>;
        // static_assert(tuple_bytes_v<type> == sizeof(type));
        // static_assert(tuple_bytes_v<type> ==);
    }


    {
        // func_ty
        
    }
}