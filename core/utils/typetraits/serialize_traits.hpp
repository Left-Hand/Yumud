#pragma once

#include "typetraits_details.hpp"
#include "size_traits.hpp"
#include "args_traits.hpp"

namespace ymd::magic{


//由于c++标准无法认定机器是小端 所以reinterpret_cast是non-constexpr的 
//这里我们强制认为机器都是小端的 使用bit_cast配合掩码进行计算
template<
    typename Arg,
    size_t N = sizeof(std::decay_t<Arg>)
>
__fast_inline constexpr 
uint8_t get_byte_from_arg(const size_t idx, Arg && arg){
    static_assert(N <= 8, "N must be less than 8");

    using T = bytes_to_uint_t<N>;

    const T raw = std::bit_cast<T>(arg);
    return uint8_t{uint8_t(raw >> (idx * 8))};
}

//由于未知原因 iq_t不支持平凡复制 故需要额外进行特化
template<size_t Q>
__fast_inline constexpr 
uint8_t get_byte_from_arg(const size_t idx, const ymd::iq_t<Q> & arg){
    static_assert(sizeof(ymd::iq_t<Q>) <= 8, "Size of iq_t<Q> must be less than 8");
    const auto raw = arg.to_i32();
    return uint8_t{uint8_t(raw >> (idx * 8))};
}


// 通过刚刚获取到的字节信息构造一个定长数组
template<
    typename Arg,
    size_t N = sizeof(std::decay_t<Arg>)
>
__fast_inline constexpr
std::array<uint8_t, N> make_bytes_from_arg(Arg && arg){
    static_assert(N <= 8, "N must be less than 8");

    return [arg]<size_t... I>(std::index_sequence<I...>) {
        return std::array<uint8_t, N>{{ get_byte_from_arg(I, arg)... }};
    }(std::make_index_sequence<N>{});
}

template<
    typename ... Args,
    size_t N = magic::total_bytes_of_args_v<std::decay_t<Args> ... >
>
__inline constexpr
std::array<uint8_t, N> serialize_args_to_bytes(Args && ... args){
    std::array<uint8_t, N> result;
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
    size_t N = magic::total_bytes_of_packed_tuple_v<std::decay_t<Tup>>
>
__inline constexpr
std::array<uint8_t, N> make_bytes_from_tuple(Tup && tup){
    std::array<uint8_t, N> result;
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
Arg make_arg_from_bytes(const std::span<const uint8_t, ArgSize> & bytes) {
    static_assert(sizeof(Arg) <= 8, "N must be less than 8"); // 确保类型不超过8字节

    using T = bytes_to_uint_t<sizeof(Arg)>; // 使用预定义的中间类型

    T temp = static_cast<T>(bytes[0]);

    for(size_t i = 1; i < sizeof(Arg); i++){
        temp |= static_cast<T>(static_cast<T>(bytes[i]) << (i * 8));
    }

    if constexpr (is_fixed_point_v<Arg>) return Arg(std::bit_cast<typename Arg::iq_type>(temp));
    else return std::bit_cast<Arg>(temp);
}


//给定指定的元组和模板 裁剪对应的元素
template<
    size_t I,
    typename Tup,
    typename Arg = std::tuple_element_t<I, Tup>,
    size_t TupSize = magic::total_bytes_of_args_v<Tup>,
    size_t ArgSize = sizeof(Arg)
>
__inline constexpr 
Arg fetch_arg_from_bytes(const std::span<const uint8_t, TupSize> bytes) {
    // 静态断言确保元组元素索引有效
    static_assert(I < std::tuple_size_v<Tup>, "out of range");

    // 计算元组元素的偏移量
    constexpr size_t offset = tuple_element_offset_v<I, Tup>;

    // 截取对应长度的子跨度（静态跨度）
    auto subspan = bytes.subspan(offset, ArgSize);

    // 使用静态跨度调用 make_arg_from_bytes
    return make_arg_from_bytes<Arg>(subspan);
}


template<typename Tup, size_t N = magic::total_bytes_of_args_v<std::decay_t<Tup>>>
__inline constexpr 
Tup make_tuple_from_bytes(const std::span<const uint8_t, N> bytes) {
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
    size_t N = magic::total_bytes_of_packed_tuple_v<ArgsTup>
>
__inline constexpr 
Ret _invoke_func_by_serialzed_bytes_impl(Fn && fn, const std::span<const uint8_t, N> bytes){
    if constexpr(std::is_void_v<Ret>) std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
    else return std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
}

// template<
//     typename Obj
//     typename Fn,
//     typename Ret,
//     typename ... Args,
//     size_t N = magic::total_bytes_of_packed_tuple_v<ArgsTup>
// >
// constexpr 
// Ret _invoke_memfunc_by_bytes_impl(Obj & obj, Ret(Obj::*fn)(Args...), const std::span<const uint8_t, N> bytes){
//     if constexpr(std::is_void_v<Ret>) std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
//     else return std::apply(std::forward<Fn>(fn), make_tuple_from_bytes<ArgsTup>(bytes));
// }

template<
    typename Obj,
    typename Ret,
    typename ... Args,
    size_t N = magic::total_bytes_of_packed_tuple_v<std::tuple<Args...>>
>
__inline constexpr 
Ret _invoke_memfunc_by_bytes_impl(
    Obj & obj,
    Ret(std::decay_t<Obj>::*fn)(Args...),
    const std::span<const uint8_t, N>& bytes
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
    size_t N = magic::total_bytes_of_packed_tuple_v<std::tuple<Args...>>
>
__inline constexpr 
Ret _invoke_memfunc_by_bytes_impl(
    const Obj & obj,
    Ret(std::decay_t<Obj>::*fn)(Args...) const,
    const std::span<const uint8_t, N>& bytes
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
    size_t N = magic::total_bytes_of_packed_tuple_v<std::tuple<Args...>>
>
constexpr Ret invoke_func_by_serialzed_bytes(Ret(*fn)(Args...), const std::span<const uint8_t, N> bytes){
    using Fn = std::decay_t<Ret(*)(Args...)>;
    return magic::details::_invoke_func_by_serialzed_bytes_impl
		<Fn, Ret, std::tuple<Args...>>(std::forward<Fn>(fn), bytes);
}


//lambda版本
template<
    typename Fn,
    typename ArgsTup = functor_args_tuple_t<std::decay_t<Fn>>,
    typename Ret = functor_ret_t<std::decay_t<Fn>>,
    size_t N = magic::total_bytes_of_packed_tuple_v<ArgsTup>
>
requires magic::is_functor_v<std::decay_t<Fn>>
constexpr Ret invoke_func_by_serialzed_bytes(Fn && fn, const std::span<const uint8_t, N> bytes){
    if constexpr(std::is_void_v<Ret>) magic::details::
		_invoke_func_by_serialzed_bytes_impl<Fn, Ret, ArgsTup>(fn, bytes);
    else return magic::details::
		_invoke_func_by_serialzed_bytes_impl<Fn, Ret, ArgsTup>(fn, bytes);
}


//成员函数版本
template<
    typename Ret, 
    typename ... Args,
    typename ArgsTup = std::tuple<Args...>,
    size_t N = magic::total_bytes_of_packed_tuple_v<ArgsTup>
>
__fast_inline constexpr Ret invoke_func_by_serialzed_bytes(
    auto & obj, 
    Ret(std::remove_reference_t<decltype(obj)>::*fn)(Args...), 
    const std::span<const uint8_t, N> bytes
){
    if constexpr(std::is_void_v<Ret>) 
        magic::details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
    else 
        return magic::details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
}

//常成员函数版本
template<
    typename Ret, 
    typename ... Args,
    typename ArgsTup = std::tuple<Args...>,
    size_t N = magic::total_bytes_of_packed_tuple_v<ArgsTup>
>
constexpr Ret invoke_func_by_serialzed_bytes(
    const auto & obj, 
    Ret(std::remove_reference_t<decltype(obj)>::*fn)(Args...) const , 
    const std::span<const uint8_t, N> bytes
){
    if constexpr(std::is_void_v<Ret>) 
        magic::details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
    else 
        return magic::details::_invoke_memfunc_by_bytes_impl<std::remove_reference_t<decltype(obj)>, Ret, Args...>(obj, fn, bytes);
}


}