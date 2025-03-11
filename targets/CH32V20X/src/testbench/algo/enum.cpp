#include "../tb.h"

#include "sys/math/real.hpp"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"
#include "sys/clock/clock.h"

// #include 
// https://taylorconor.com/blog/enum-reflection/


template <size_t Size>
struct bytes_to_uint;

template <>
struct bytes_to_uint<1> {
    using type = uint8_t;
};

template <>
struct bytes_to_uint<2> {
    using type = uint16_t;
};

template <>
struct bytes_to_uint<4> {
    using type = uint32_t;
};

template<size_t Bytes>
using bytes_to_uint_t = typename bytes_to_uint<Bytes>::type;

template<size_t Bits>
using bits_to_uint_t = typename bytes_to_uint<(Bits + 7) / 8>::type;

template<typename T>
static constexpr size_t type_to_bits_v = sizeof(T) * 8;

template<typename T>
static constexpr size_t type_to_bytes_v = sizeof(T) * 8;


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
    struct _total_bytes_v{
        static constexpr size_t value = _total_bytes_v<N-1, Args...>::value + sizeof(typename _element_t<N, Args...>::type);
    };

    template<typename ... Args>
    struct _total_bytes_v<0, Args ...>{
        static constexpr size_t value = sizeof(typename _element_t<0, Args...>::type);
    };

    template<typename T>
    struct _packed_tuple_total_bytes_v{};

    template<typename ... Args>
    struct _packed_tuple_total_bytes_v<std::tuple<Args...>>{
        static constexpr size_t value = _total_bytes_v<sizeof...(Args) - 1, Args...>::value;
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

    template<class, class = void>
    struct is_functor : std::false_type {};

    template<class T>
    struct is_functor<T, std::void_t<decltype(&T::operator())>> : std::true_type {};


    template<typename T>
    struct function_traits;


    template<typename Functor, typename Ret, typename... Args>
    struct function_traits<Ret(Functor::*)(Args...) const> {
        using return_type = Ret;
        using args_type = std::tuple<Args...>;
    };

    template<typename Functor, typename... Args>
    struct function_traits<void(Functor::*)(Args...) const> {
        using return_type = void;
        using args_type = std::tuple<Args...>;
    };
    // template<class T>
    // struct is_function_pointer
    // : bool_constant<std::is_pointer_v<T> && is_function_v<remove_pointer_t<T>>> {};


    template <typename Functor>
    requires is_functor<std::decay_t<Functor>>::value
    struct functor_traits {
    private:
        using call_type = function_traits<decltype(&Functor::operator())>;
    public:
        using return_type = typename call_type::return_type;
        using args_type = typename call_type::args_type;
    };


}

using namespace details;


template<typename T>
static constexpr bool is_functor_v = is_functor<T>::value;

// template<typename T>
// static constexpr bool is_fpointer_v = details::is_function_pointer<T>::value;

template <typename E, E V> 
constexpr const char * enum_item_name_v = _enum_item_name<E, V>();


template <typename E, E V> 
constexpr bool enum_is_valid_v = _enum_is_valid<E, V>();


template <typename E>
constexpr int enum_count_v = _enum_internal_element_count<E>(std::make_integer_sequence<int, 100>());


template<size_t N, typename Tup>
static constexpr size_t tuple_element_offset_v = _tuple_element_offset_v<N, Tup>::value;



template<size_t N, typename ... Args>
using element_t = typename _element_t<N, Args...>::type;

template<size_t N, typename ... Args>
static constexpr size_t element_bytes_v = sizeof(element_t<N, Args...>);


template<size_t N, typename Tup>
using tuple_element_t = std::decay_t<typename _tuple_element_t<N, Tup>::type>;





template<typename Fn>
using functor_ret_t = typename details::functor_traits<Fn>::return_type;

template<typename Fn>
using functor_args_tuple_t = typename details::functor_traits<Fn>::args_type;


template<typename ... Args>
static constexpr size_t total_bytes_v = _total_bytes_v<sizeof...(Args) - 1, Args...>::value;


template<typename Tup>
static constexpr size_t packed_tuple_total_bytes_v = _packed_tuple_total_bytes_v<Tup>::value;






enum Fruit {
    BANANA = 5,
    APPLE = 12,
};




//由于c++标准无法认定机器是小端 所以reinterpret_cast是non-constexpr的 
//这里我们强制认为机器都是小端的 使用bit_cast配合掩码进行计算
template<
    typename Arg,
    size_t N = sizeof(std::decay_t<Arg>)
>
__fast_inline constexpr 
std::byte get_byte_from_arg(const size_t idx, Arg && arg){
    static_assert(N <= 8, "N must be less than 8");

    using T = bytes_to_uint_t<N>;

    const T raw = std::bit_cast<T>(arg);
    return std::byte{uint8_t(raw >> (idx * 8))};
}

//由于未知原因 iq_t不支持平凡复制 故需要额外进行特化
template<size_t Q>
__fast_inline constexpr 
std::byte get_byte_from_arg(const size_t idx, const ymd::iq_t<Q> & arg){
    static_assert(sizeof(ymd::iq_t<Q>) <= 8, "Size of iq_t<Q> must be less than 8");

    using T = bytes_to_uint_t<sizeof(ymd::iq_t<Q>)>;

    const T raw = std::bit_cast<T>(arg.value);
    return std::byte{uint8_t(raw >> (idx * 8))};
}


// 通过刚刚获取到的字节信息构造一个定长数组
template<
    typename Arg,
    size_t N = sizeof(std::decay_t<Arg>)
>
__fast_inline constexpr
std::array<std::byte, N> make_bytes_from_arg(Arg && arg){
    static_assert(N <= 8, "N must be less than 8");

    return [arg]<size_t... I>(std::index_sequence<I...>) {
        return std::array<std::byte, N>{{ get_byte_from_arg(I, arg)... }};
    }(std::make_index_sequence<N>{});
}



// template<size_t N1, size_t N2>
// constexpr
// std::array<std::byte, N1 + N2> concat_arr(
//     const std::array<const std::byte, N1> & arr1,
//     const std::array<const std::byte, N2> & arr2
// ){
//     return [&](){
//         std::array<std::byte, N1 + N2> ret;
//         for(size_t i = 0; i < N1; i++)
//             ret[i] = arr1[i];
//         for(size_t i = 0; i < N2; i++)
//             ret[N1 + i] = arr2[i];
//         return ret;
//     }();
// }


template<
    typename ... Args,
    size_t N = total_bytes_v<std::decay_t<Args> ... >
>
__inline constexpr
std::array<std::byte, N> make_bytes_from_args(Args && ... args){
    std::array<std::byte, N> result;
    size_t offset = 0;

    ([&](const auto& arg) {
        constexpr size_t arg_size = sizeof(std::decay_t<decltype(arg)>);
        auto arg_bytes = make_bytes_from_arg(arg);
        std::copy(arg_bytes.begin(), arg_bytes.end(), result.begin() + offset);
        offset += arg_size;
    }(args), ...);

    return result;
};


template<
    typename Tup,
    size_t N = packed_tuple_total_bytes_v<std::decay_t<Tup>>
>
__inline constexpr
std::array<std::byte, N> make_bytes_from_tuple(Tup && tup){
    std::array<std::byte, N> result;
    size_t offset = 0;

    std::apply([&result, &offset](auto&&... args) {
        ([&](const auto& arg) {
            constexpr size_t arg_size = sizeof(std::decay_t<decltype(arg)>);
            auto arg_bytes = make_bytes_from_arg(arg);
            std::copy(arg_bytes.begin(), arg_bytes.end(), result.begin() + offset);
            offset += arg_size;
        }(args), ...);
    }, std::forward<Tup>(tup));

    return result;
};



template<
    typename _Arg,
    typename Arg = std::decay_t<_Arg>,
    size_t ArgSize = sizeof(Arg)
>
__fast_inline constexpr 
Arg make_arg_from_bytes(const std::span<const std::byte, ArgSize> & bytes) {
    static_assert(sizeof(Arg) <= 8, "N must be less than 8"); // 确保类型不超过8字节

    using T = bytes_to_uint_t<sizeof(Arg)>; // 使用预定义的中间类型

    T temp = static_cast<T>(bytes[0]);

    for(size_t i = 1; i < sizeof(Arg); i++){
        temp |= static_cast<T>(static_cast<T>(bytes[i]) << (i * 8));
    }

    if constexpr (is_fixed_point_v<Arg>) return Arg(std::bit_cast<typename Arg::iq_type>(temp));
    else return std::bit_cast<Arg>(temp);
}

template<
    size_t I,
    typename Tup,
    typename Arg = tuple_element_t<I, Tup>,
    size_t TupSize = total_bytes_v<Tup>,
    size_t ArgSize = sizeof(Arg)
>
__inline constexpr 
Arg fetch_arg_from_bytes(const std::span<const std::byte, TupSize> bytes) {
    // 静态断言确保元组元素索引有效
    static_assert(I < std::tuple_size_v<Tup>, "out of range");

    // 计算元组元素的偏移量
    constexpr size_t offset = tuple_element_offset_v<I, Tup>;

    // 截取对应长度的子跨度（静态跨度）
    auto subspan = bytes.subspan(offset, ArgSize);

    // 使用静态跨度调用 make_arg_from_bytes
    return make_arg_from_bytes<Arg>(subspan);
}


template<typename Tup, size_t N = total_bytes_v<std::decay_t<Tup>>>
__inline constexpr 
Tup make_tuple_from_bytes(const std::span<const std::byte, N> bytes) {
    using Is = std::make_index_sequence<std::tuple_size_v<Tup>>;

    // 根据索引序列展开并构造元组
    auto construct_tuple = [&bytes]<std::size_t... Indices>(std::index_sequence<Indices...>) {
        return std::make_tuple(fetch_arg_from_bytes<Indices, Tup>(bytes)...);
    };

    // 使用索引序列调用辅助函数
    return construct_tuple(Is{});
}

namespace details{
template<
    typename Fn,
    typename Ret,
    typename ArgsTup,
    size_t N = packed_tuple_total_bytes_v<ArgsTup>
>
__inline constexpr 
Ret _invoke_func_by_bytes_impl(Fn && fn, const std::span<const std::byte, N> bytes){
    if constexpr(std::is_void_v<Ret>) std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
    else return std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
}

// template<
//     typename Obj
//     typename Fn,
//     typename Ret,
//     typename ... Args,
//     size_t N = packed_tuple_total_bytes_v<ArgsTup>
// >
// constexpr 
// Ret _invoke_memfunc_by_bytes_impl(Obj & obj, Ret(Obj::*fn)(Args...), const std::span<const std::byte, N> bytes){
//     if constexpr(std::is_void_v<Ret>) std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
//     else return std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
// }

template<
    typename Obj,
    typename Ret,
    typename ... Args,
    size_t N = packed_tuple_total_bytes_v<std::tuple<Args...>>
>
__inline constexpr 
Ret _invoke_memfunc_by_bytes_impl(
    Obj & obj,
    Ret(std::decay_t<Obj>::*fn)(Args...),
    const std::span<const std::byte, N>& bytes
) {
    auto args_tuple = make_tuple_from_bytes<std::tuple<Args...>>(bytes);
    if constexpr (std::is_void_v<Ret>) {
        std::apply([&](Args... args) { (obj.*fn)(args...); }, args_tuple);
    } else {
        return std::apply([&](Args... args) { return (obj.*fn)(args...); }, args_tuple);
    }
}

template<
    typename Obj,
    typename Ret,
    typename ... Args,
    size_t N = packed_tuple_total_bytes_v<std::tuple<Args...>>
>
__inline constexpr 
Ret _invoke_memfunc_by_bytes_impl(
    const Obj & obj,
    Ret(std::decay_t<Obj>::*fn)(Args...) const,
    const std::span<const std::byte, N>& bytes
) {
    auto args_tuple = make_tuple_from_bytes<std::tuple<Args...>>(bytes);
    if constexpr (std::is_void_v<Ret>) {
        std::apply([&](Args... args) { (obj.*fn)(args...); }, args_tuple);
    } else {
        return std::apply([&](Args... args) { return (obj.*fn)(args...); }, args_tuple);
    }
}

}

template<
    typename Ret,
    typename ... Args,
    size_t N = packed_tuple_total_bytes_v<std::tuple<Args...>>
>
constexpr Ret invoke_func_by_bytes(Ret(*fn)(Args...), const std::span<const std::byte, N> bytes){
    using Fn = std::decay_t<Ret(*)(Args...)>;
    return _invoke_func_by_bytes_impl<Fn, Ret, std::tuple<Args...>>(std::forward<Fn>(fn), bytes);
}


//lambda版本
template<
    typename Fn,
    typename ArgsTup = functor_args_tuple_t<std::decay_t<Fn>>,
    typename Ret = functor_ret_t<std::decay_t<Fn>>,
    size_t N = packed_tuple_total_bytes_v<ArgsTup>
>
requires is_functor_v<std::decay_t<Fn>>
constexpr Ret invoke_func_by_bytes(Fn && fn, const std::span<const std::byte, N> bytes){
    if constexpr(std::is_void_v<Ret>) details::_invoke_func_by_bytes_impl<Fn, Ret, ArgsTup>(fn, bytes);
    else return details::_invoke_func_by_bytes_impl<Fn, Ret, ArgsTup>(fn, bytes);
}


//成员函数版本
template<
    typename Ret, 
    typename ... Args,
    typename ArgsTup = std::tuple<Args...>,
    size_t N = packed_tuple_total_bytes_v<ArgsTup>
>
__fast_inline constexpr Ret invoke_func_by_bytes(
    auto & obj, 
    Ret(std::remove_reference_t<decltype(obj)>::*fn)(Args...), 
    const std::span<const std::byte, N> bytes
){
    if constexpr(std::is_void_v<Ret>) 
        details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
    else 
        return details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
}

//常成员函数版本
template<
    typename Ret, 
    typename ... Args,
    typename ArgsTup = std::tuple<Args...>,
    size_t N = packed_tuple_total_bytes_v<ArgsTup>
>
constexpr Ret invoke_func_by_bytes(
    const auto & obj, 
    Ret(std::remove_reference_t<decltype(obj)>::*fn)(Args...) const , 
    const std::span<const std::byte, N> bytes
){
    if constexpr(std::is_void_v<Ret>) 
        details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
    else 
        return details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
}

auto pfunc(const real_t a, const real_t b){
    return a * a + b * b;
    // return a;
};

auto pfunc(const int16_t a, const int16_t b){
    return int(a) * a + int(b) * b;
    // return a;
};

class Ball{
public:
    int a;
    iq_t<16> mset_xy(iq_t<16> x, iq_t<16> y){
        return x * x + y * y;
    }

    iq_t<16> mset_xy2(iq_t<16> x, iq_t<16> y){
        return x + y;
    }
};

// static constexpr bool test_if = is_functor_v<decltype(pfunc)>;

auto func = [](const iq_t<30> a, iq_t<30> b){
    return a * a + b * b;
    // return a;
};

auto runtime_true(){
    return sys::Chip::getChipIdCrc();
}



void enum_main(){

    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    Ball ball;

    while(true){
        // auto set_xy = [](iq_t<16> x, iq_t<16> y){
        //     return x * x + y * y;
        // };

        const auto t = time();

        const auto begin_m = micros();
        // const auto s = sin<30>(t);
        // const auto c = cos<30>(t);
        const auto s = sin(t);
        const auto c = cos(t);
        // const auto s = int16_t(sin(t) * 300);
        // const auto c = int16_t(cos(t) * 300);
        
        real_t r = 0; 
        // int r = 0; 

        auto dyn_func = runtime_true() ? &Ball::mset_xy : &Ball::mset_xy2;
        const auto bytes = make_bytes_from_args(s, c);
        for(size_t i = 0; i < 100000; i++){
            // r = invoke_func_by_bytes(set_xy, std::span(bytes));
            // r = invoke_func_by_bytes<int, int16_t, int16_t>(pfunc, std::span(bytes));
            // r = invoke_func_by_bytes<int>(pfunc, std::span(bytes));
            // r = invoke_func_by_bytes<int>(pfunc, std::span(bytes));
            // r = invoke_func_by_bytes(ball, &Ball::mset_xy, std::span(bytes));
            r = invoke_func_by_bytes(ball, dyn_func, std::span(bytes));
        }
        // DEBUG_PRINTLN(t);
        DEBUG_PRINTLN(s,c,r, uint32_t(micros() - begin_m));
        delay(10);
        // delay(10);
    }


    {
        [[maybe_unused]] constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
        [[maybe_unused]] constexpr const char * _10 = enum_item_name_v<Fruit, Fruit(10)>;

        [[maybe_unused]] constexpr size_t count = enum_count_v<Fruit>;
        // constexpr const char * banana2 = enum_item_name_v2<Fruit::BANANA>;
        // using type = decltype(Fruit::BANANA);
        // constexpr const char * banana = enum_item_name_v<Fruit, Fruit::BANANA>;
    
        // static_assert(!, "!enum_is_valid_v<Fruit, Fruit::10>()");
        // const auto f = __PRETTY_FUNCTION__;
        static_assert(enum_is_valid_v<Fruit, Fruit::BANANA>, "enum_is not _valid_v<Fruit, Fruit::BANANA>()");
        static_assert(!enum_is_valid_v<Fruit, Fruit(10)>, "enum_is_valid_v<Fruit, Fruit::10>()");
        // static_assert(!enum_is_valid_v<Fruit, Fruit>, "!enum_is_valid_v<Fruit, Fruit::10>()");
    }


    // static constexpr size_t a = tuple_bytes_v<int, int, uint8_t>;
    {
        // using tup_t = std::tuple<int, uint16_t, uint8_t, iq_t<16>>;
        
        // static constexpr size_t a1 = tuple_element_bytes_v<1, tup_t>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, tup_t>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, tup_t>;

        // static constexpr size_t a3 = element_bytes_v<2, int, uint16_t, uint8_t, iq_t<16>>;
        using t0 = element_t<0, int, uint16_t, uint8_t, iq_t<16>>;
        using t1 = element_t<1, int, uint16_t, uint8_t, iq_t<16>>;
        
        using t3 = tuple_element_t<3, std::tuple<int, uint16_t, uint8_t, iq_t<16> >>;

        static_assert(std::is_same_v<t0, int>);
        static_assert(std::is_same_v<t1, uint16_t>);
        static_assert(std::is_same_v<t3, iq_t<16>>);

        // static constexpr auto tup1 = std::make_tuple<int, uint16_t, uint8_t, iq_t<16> >(1, 2, 3, 4.0_r);
        
        // static_assert(packed_tuple_total_bytes_v<tup_t> == sizeof(tup_t));
        // total_bytes_v<int, uint16_t, uint8_t, iq_t<16>>;
        // packed_tuple_total_bytes_v<tup_t>;
        // static constexpr size_t a0 = element_bytes_v<0, int, uint16_t, uint8_t, iq_t<16>>;
        // static constexpr size_t a1 = element_bytes_v<1, int, uint16_t, uint8_t, iq_t<16>>;
        
        // static constexpr size_t a = element_bytes_v<1, int, short>;
        // static_assert(tuple_bytes_v<tup_t> == sizeof(tup_t));
        // static_assert(tuple_bytes_v<tup_t> ==);
    }
    
    
    {
        // func_ty
        static constexpr uint32_t u = 0x12345678;
        
        static_assert(uint8_t(get_byte_from_arg(0, u)) == 0x78);
        static_assert(uint8_t(get_byte_from_arg(1, u)) == 0x56);
        static_assert(uint8_t(get_byte_from_arg(2, u)) == 0x34);
        static_assert(uint8_t(get_byte_from_arg(3, u)) == 0x12);

        static constexpr float f = 1.234f;

        static_assert(uint8_t(get_byte_from_arg(0, f)) == 0xB6);
        static_assert(uint8_t(get_byte_from_arg(1, f)) == 0xF3);
        static_assert(uint8_t(get_byte_from_arg(2, f)) == 0x9D);
        static_assert(uint8_t(get_byte_from_arg(3, f)) == 0x3F);

        static constexpr auto arr1 = make_bytes_from_arg(u);
        static constexpr auto arr2 = make_bytes_from_arg(f);

        static constexpr auto u_ = make_arg_from_bytes<decltype(u)>(std::span(arr1));
        static constexpr auto f_ = make_arg_from_bytes<decltype(f)>(std::span(arr2));

        static_assert(u == u_);
        static_assert(f == f_);

        static constexpr uint16_t ui16 = 0xabcd;
        static constexpr auto q21 = 0.12_q21;
        // static constexpr auto q21 = 16;

        static constexpr auto arr3 = make_bytes_from_args(u, f, ui16, q21);
        static constexpr auto arr4 = make_bytes_from_tuple(std::make_tuple(u, f, ui16, q21));
        static_assert(arr3 == arr4);

        static constexpr auto tup = std::make_tuple(u, f, ui16, q21);
        using tup_t = std::decay_t<decltype(tup)>;

        // [[maybe_unused]] using a0 = tuple_element_t<0, tup_t>;
        // [[maybe_unused]] using a1 = tuple_element_t<1, tup_t>;
        // [[maybe_unused]] using a2 = tuple_element_t<2, tup_t>;

        static constexpr auto u_2 = fetch_arg_from_bytes<0, tup_t>(std::span(arr3));
        static constexpr auto f_2 = fetch_arg_from_bytes<1, tup_t>(std::span(arr3));

        static_assert(u == u_2);
        static_assert(f == f_2);
        // static constexpr auto f_2 = fetch_arg_from_bytes<1, tup_t>(std::span(arr3));

        // static_ass
        static constexpr auto tup_ = make_tuple_from_bytes<tup_t>(std::span<const std::byte, 14>(arr4));
        static_assert(tup == tup_);
        // static const auto make_arg_from_bytes
    }

    {
        // constexpr 
        auto func1 = [](const uint16_t u, const iq_t<10> q){
            return iq_t<20>(q + u);
        };

        static constexpr uint16_t u1 = 10;
        static constexpr iq_t<10> q1 = 0.1_r;
        static constexpr auto r1 = func1(u1, q1);

        static constexpr auto bytes = make_bytes_from_args(u1, q1);
        
        static_assert(is_functor_v<decltype(func1)>);

        using traits = functor_traits<std::decay_t<decltype(func1)>>;
        static_assert(std::is_same_v<traits::return_type, iq_t<20>>);
        static constexpr auto r2 = invoke_func_by_bytes(func1, std::span(bytes));
        static_assert(r1 == r2);
    }
}