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
