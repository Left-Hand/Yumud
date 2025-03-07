#pragma once


namespace details{
template<typename T>
struct _is_member_variable{
    static constexpr bool value = false;
};

template<typename T, typename U>
struct _is_member_variable<U T::*>{
    static constexpr bool value = true;
};

template<typename T>
struct _is_member_function{
    static constexpr bool value = false;
};

template<typename T, typename U>
struct _is_member_function<U T::*>{
    static constexpr bool value = false;
};

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


}

// template<typename T>
// static constexpr bool is_member_variable_v = details::_is_member_variable<decltype(&T::)>::value;


template<typename T>
static constexpr bool is_member_variable_v = details::_is_member_variable<decltype(&T::)>::value;

template<typename T>
static constexpr bool is_member_function_v = details::_is_member_function<decltype(&T::)>::value;


// 只有在特化时才能够确定值 可用于constexpr排除分支下的static_assert
template <typename T>
static constexpr bool false_v = false;

template <typename T>
static constexpr bool true_v = true;