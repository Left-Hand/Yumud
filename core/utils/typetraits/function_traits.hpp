// #pragma once

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



// 自定义 function_traits，直接提取 lambda 的 operator() 的签名
template<typename T>
struct function_traits;

// 特化：匹配 lambda 表达式的 operator()
template<typename Functor>
struct function_traits {
private:
    using call_type = function_traits<decltype(&Functor::operator())>;
public:
    using return_type = typename call_type::return_type;
    using args_type = typename call_type::args_type;
};

// 特化：匹配 lambda 的 operator() 的签名
template<typename Functor, typename Ret, typename... Args>
struct function_traits<Ret(Functor::*)(Args...) const> {
    using return_type = Ret;
    using args_type = std::tuple<Args...>;
};


template<char...args>
struct __static_string
{
    static constexpr const char str[]={args...};
    operator const char*()const{return __static_string::str;}
};

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
		constexpr auto str=__static_string<type_name_view[indices]...,'\0'>();
		return str;
	}(indices);
	return type_name;
}