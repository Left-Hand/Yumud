#include "../tb.h"

#include "sys/math/real.hpp"

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

}

using namespace details;



template <typename E, E V> 
constexpr const char * enum_item_name_v = _enum_item_name<E, V>();


template <typename E, E V> 
constexpr bool enum_is_valid_v = _enum_is_valid<E, V>();


template <typename E>
constexpr int enum_count_v = _enum_internal_element_count<E>(std::make_integer_sequence<int, 100>());


// template <size_t N, typename T>
// struct _tuple_element_offset_v{
// };


template <size_t N, typename Tup>
struct _tuple_element_offset_v{
    static constexpr size_t value = 
        _tuple_element_offset_v<N - 1, Tup>::value + sizeof(std::tuple_element_t<N - 1, Tup>);
};


template<typename Tup>
struct _tuple_element_offset_v<0, Tup>{
    static constexpr size_t value = 0;
};

template<size_t N, typename Tup>
static constexpr size_t tuple_element_offset_v = _tuple_element_offset_v<N, Tup>::value;



template<size_t N, typename ... Args>
using element_t = typename _element_t<N, Args...>::type;

template<size_t N, typename ... Args>
static constexpr size_t element_bytes_v = sizeof(element_t<N, Args...>);


template<size_t N, typename Tup>
using tuple_element_t = std::decay_t<typename _tuple_element_t<N, Tup>::type>;


template<typename T>
struct function_traits;

template<typename Functor>
struct function_traits {
private:
    using call_type = function_traits<decltype(&Functor::operator())>;
public:
    using return_type = typename call_type::return_type;
    using args_type = typename call_type::args_type;
};

template<typename Functor, typename Ret, typename... Args>
struct function_traits<Ret(Functor::*)(Args...) const> {
    using return_type = Ret;
    using args_type = std::tuple<Args...>;
};

template<typename Fn>
using func_return_t = typename function_traits<Fn>::return_type;

template<typename Fn>
using func_args_tuple_t = typename function_traits<Fn>::args_type;


template<typename ... Args>
static constexpr size_t total_bytes_v = _total_bytes_v<sizeof...(Args) - 1, Args...>::value;


template<typename Tup>
static constexpr size_t packed_tuple_total_bytes_v = _packed_tuple_total_bytes_v<Tup>::value;

template<typename T>
static constexpr bool is_functor_v = requires(T t){t();};

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


//通过刚刚获取到的字节信息构造一个定长数组
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



template<size_t N1, size_t N2>
constexpr
std::array<std::byte, N1 + N2> concat_arr(
    const std::array<const std::byte, N1> & arr1,
    const std::array<const std::byte, N2> & arr2
){
    return [&](){
        std::array<std::byte, N1 + N2> ret;
        for(size_t i = 0; i < N1; i++)
            ret[i] = arr1[i];
        for(size_t i = 0; i < N2; i++)
            ret[N1 + i] = arr2[i];
        return ret;
    }();
}


template<
    typename ... Args,
    size_t N = total_bytes_v<std::decay_t<Args> ... >
>
constexpr
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
constexpr
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
    size_t N = sizeof(Arg)
>
__fast_inline constexpr 
Arg make_arg_from_bytes(const std::span<const std::byte, N> & bytes) {
    static_assert(N <= 8, "N must be less than 8"); // 确保类型不超过8字节

    using T = bytes_to_uint_t<N>; // 使用预定义的中间类型

    T temp = static_cast<T>(bytes[0]);

    for(size_t i = 1; i < N; i++){
        temp |= static_cast<T>(static_cast<T>(bytes[i]) << (i * 8));
    }

    return std::bit_cast<Arg>(temp);
}

template<
    size_t N_,
    typename Tup,
    typename Arg = tuple_element_t<N_, Tup>,
    size_t TupSize = total_bytes_v<Tup>,
    size_t ArgSize = sizeof(Arg)
>
constexpr 
Arg fetch_arg_from_bytes(const std::span<const std::byte> bytes){
    // constexpr size_t arg_size = sizeof(Arg);
    // const auto cut_span = bytes.subspan(tuple_element_offset_v<N_, Tup>, ArgSize);
    // static_assert(ArgSize == cut_span);
    // return make_arg_from_bytes<Arg>(bytes[tuple_element_offset_v<N_, Tup>]);
    constexpr size_t offset = tuple_element_offset_v<N_, Tup>;
    static_assert(offset == 0);
    return make_arg_from_bytes<Arg>(bytes.subspan(offset, ArgSize));
};


// template<
//     typename Tup,
//     size_t N = total_bytes_v<std::decay_t<Tup>>
// >
// constexpr 
// Tup make_tuple_from_packed_bytes(const std::span<const std::byte, N> bytes) {



//     using  Is = std::make_index_sequence<std::tuple_size_v<Tup>>;
//     // 使用索引序列展开元组元素类型
//     return std::make_tuple( fetch_arg_from_bytes.template operator()<Is>()...);
// }



template<
    typename Fn,
    typename Ret = func_return_t<std::decay_t<Fn>>,
    typename Args = func_args_tuple_t<std::decay_t<Fn>>,
    size_t N = packed_tuple_total_bytes_v<Args>
>

constexpr 
void invoke_func_with_bytes(Fn && fn, const std::span<std::byte, N> bytes){

}

void enum_main(){
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

    // static constexpr size_t a = tuple_bytes_v<int, int, uint8_t>;
    {
        using tup_t = std::tuple<int, uint16_t, uint8_t, iq_t<16>>;
        
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
        // static constexpr auto q21 = 0.12_q21;
        static constexpr auto q21 = 16;

        static constexpr auto arr3 = make_bytes_from_args(u, f, ui16, q21);
        static constexpr auto arr4 = make_bytes_from_tuple(std::make_tuple(u, f, ui16, q21));
        static_assert(arr3 == arr4);

        using tup_t = decltype(std::make_tuple(u, f, ui16, q21));

        using a0 = tuple_element_t<0, tup_t>;
        using a1 = tuple_element_t<1, tup_t>;
        using a2 = tuple_element_t<2, tup_t>;

        static constexpr auto u_2 = fetch_arg_from_bytes<0, tup_t>(std::span(arr3));


        // static constexpr auto f_2 = fetch_arg_from_bytes<1, tup_t>(std::span(arr3));

        // static_ass
        // static constexpr auto arr_ = make_tuple_from_packed_bytes<tup_t>(std::span(arr4));
        // static const auto make_arg_from_bytes
    }

    {

    }
}