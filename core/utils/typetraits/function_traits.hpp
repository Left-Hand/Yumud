#pragma once

#include "typetraits_details.hpp"
#include "serialize_traits.hpp"

// #include <functional>
// #include <type_traits>

// // 通用模板，用于匹配普通函数、函数指针和 lambda
// template<typename T>
// struct function_traits;

// // 特化：匹配普通函数和函数指针
// template<typename Ret, typename... Args>
// struct function_traits<Ret(Args...)> {
//     using return_type = Ret;
//     using args_type = std::tuple<Args...>;
// };

// // 特化：匹配函数指针
// template<typename Ret, typename... Args>
// struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

// // 特化：匹配 lambda 表达式和 std::function
// template<typename Functor>
// struct function_traits {
// private:
//     using call_type = function_traits<decltype(&Functor::operator())>;
// public:
//     using return_type = typename call_type::return_type;
//     using args_type = typename call_type::args_type;
// };

// // 特化：匹配 lambda 的 operator()
// template<typename Functor, typename Ret, typename... Args>
// struct function_traits<Ret(Functor::*)(Args...) const> : function_traits<Ret(Args...)> {};


// // 辅助模板：将 std::tuple 展开为参数包
// template<typename Ret, typename ArgsTuple, template<typename, typename...> class MethodByLambda>
// struct make_method_by_lambda;

// template<typename Ret, template<typename, typename...> class MethodByLambda, typename... Args>
// struct make_method_by_lambda<Ret, std::tuple<Args...>, MethodByLambda> {
//     using type = MethodByLambda<Ret, Args...>;
// };

namespace ymd::magic{

namespace details{
	template<class, class = void>
	struct _is_functor : std::false_type {};

	template<class T>
	struct _is_functor<T, std::void_t<decltype(&T::operator())>> : std::true_type {};


	template<typename T>
	struct _function_call_type;


	template<typename Functor, typename Ret, typename... Args>
	struct _function_call_type<Ret(Functor::*)(Args...) const> {
		using return_type = Ret;
		using args_type = std::tuple<Args...>;
	};

	template<typename Functor, typename... Args>
	struct _function_call_type<void(Functor::*)(Args...) const> {
		using return_type = void;
		using args_type = std::tuple<Args...>;
	};

	// template<class T>
	// struct is_function_pointer
	// : bool_constant<std::is_pointer_v<T> && is_function_v<remove_pointer_t<T>>> {};


	template <typename Functor>
	requires _is_functor<std::decay_t<Functor>>::value
	struct _functor_traits {
	private:
		using call_type = _function_call_type<decltype(&Functor::operator())>;
	public:
		using return_type = typename call_type::return_type;
		using args_type = typename call_type::args_type;
	};

}

template<typename T>
static constexpr bool is_functor_v = details::_is_functor<T>::value;

template<typename Fn>
using functor_ret_t = typename details::_functor_traits<Fn>::return_type;

template<typename Fn>
using functor_args_tuple_t = typename details::_functor_traits<Fn>::args_type;


template<typename Object>
constexpr auto type_name_of(){


	constexpr std::string_view fully_name=__PRETTY_FUNCTION__;		
	constexpr std::size_t begin=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]=='=')
				return i+2;
	}();
	constexpr std::size_t end=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]==']')
				return i;
	}();
	constexpr auto type_name_view=fully_name.substr(begin,end-begin);
	constexpr auto indices=std::make_index_sequence<type_name_view.size()>();
	constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
	{
		constexpr auto str = details::__static_string<type_name_view[indices]...,'\0'>();
		return str;
	}(indices);
	return type_name;
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