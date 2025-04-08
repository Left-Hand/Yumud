#pragma once

#include "core/platform.hpp"
#include "core/debug/debug.hpp"
#include "details/unwrap_util.hpp"
#include <variant>
#include <source_location>

namespace ymd{


namespace custom{
    template <typename T, typename S> 
    struct option_converter{};

    // 非侵入式地添加隐式类型转换
    // T为正确类型 S为源类型

    // eg:
    // template<>
    // struct option_converter<BusError, BusError> {
    //     static Option<BusError> convert(const BusError & res){
    //         if(res.ok()) return Some(res);
    //         else return None; 
    //     }
    // };
}


template<typename T>
static constexpr bool is_option_v = false;

template<typename T>
static constexpr bool is_option_v<Option<T>> = true;

namespace details{
template<typename T>
struct _option_type{
};

template<typename T>
struct _option_type<Option<T>>{
    using type = T;
};

}

template<typename T>
using option_type_t = details::_option_type<T>::type;

template<typename T>
class Option{
private:
    using data_t = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
    T value_;
    bool exists_;
public:
    [[nodiscard]] constexpr 
    Option(_None_t):
        exists_(false)
    {}

    [[nodiscard]] constexpr 
    Option(const Some<T> & value):
        value_(value),
        exists_(true)
        {}

    [[nodiscard]] constexpr 
    Option(Some<T> && value):
        value_(std::move(*value)),
        exists_(true)
        {}


    template<typename S>
    requires requires(S s) {
        { custom::option_converter<T, S>::convert(s) } -> std::convertible_to<Option<T>>;
    }
    [[nodiscard]] __fast_inline constexpr 
    Option(const S & other):Option(custom::option_converter<T, S>::convert(other)){}
    [[nodiscard]] __fast_inline constexpr bool 
    is_some() const{ return exists_; }
    [[nodiscard]] __fast_inline constexpr bool 
    is_none() const{ return !exists_; }

    [[nodiscard]] __fast_inline constexpr const T & 
    value_or(const T & default_value) const{
        return exists_ ? value_ : default_value;
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    unwrap_loc(const std::source_location & loc = std::source_location::current()) const {
        if(unlikely(exists_ == false)){
            __PANIC_EXPLICIT_SOURCE(loc);
        }
        return value_;
    }

    [[nodiscard]] __fast_inline constexpr const T & 
    unwrap() const {
        if(unlikely(exists_ == false)){
            exit(1);
        }
        return value_;
    }

    [[nodiscard]] __fast_inline constexpr T 
    operator +() const{
        return unwrap();
    }


    __fast_inline constexpr void 
    unexpected() const {
        return;
    }

    // // 逻辑组合运算符
    // template<typename U>
    // Option<T> operator||(Option<U> lhs, Option<T> rhs) {
    //     return lhs.is_some() ? lhs : rhs;
    // }

    // template<typename U>
    // Option<T> operator&&(Option<U> lhs, Option<T> rhs) {
    //     return lhs.is_some() ? rhs : lhs;
    // }

    template<typename FnSome, typename FnNone>
    constexpr auto match(FnSome&& some_fn, FnNone&& none_fn) const& {
        if (is_some()) {
            return std::invoke(std::forward<FnSome>(some_fn), value_);
        } else {
            return std::invoke(std::forward<FnNone>(none_fn));
        }
    }

    // 函数式映射 (Monadic map)
    template<
        typename Fn,
        typename TIResult = std::invoke_result_t<Fn, T>
    >
    constexpr auto map(Fn&& fn) const& -> Option<TIResult> {
        if (is_some()) {
            return Some<TIResult>(std::forward<Fn>(fn)(value_));
        }
        return None;
    }

    // 链式操作 (Monadic and_then)
    template<
        typename Fn, 
        typename TIResult = std::invoke_result_t<Fn, T>, 
        typename TData = std::conditional_t<
            is_option_v<TIResult>, 
            option_type_t<TIResult>, 
            TIResult
        >
    >

    constexpr auto and_then(Fn&& fn) const -> Option<TData>{
        if (is_some()){
            if constexpr(is_option_v<TIResult>){
                return std::forward<Fn>(fn)(value_);
            }else{
                return Some<TData>(std::forward<Fn>(fn)(value_));
            }
        }
        return None;
    }

    // // 安全空值传播
    template<typename Fn>
    requires requires(Fn fn, T val) { 
        { fn(val) } -> std::same_as<Option<typename std::invoke_result_t<Fn, T>>>; 
    }
    constexpr auto operator>>=(Fn&& fn) const {
        return this->and_then(std::forward<Fn>(fn));
    }

    // template<typename U, typename Fn>
    // friend auto operator|(const Option<U>& opt, Fn&& fn);

    template<typename Fn>
    __fast_inline constexpr 
    const Option<T> & inspect(Fn && fn) const {
        if (is_some()) std::forward<Fn>(fn)(unwrap());
        return *this;
    }
};


template<std::size_t I, typename T>
auto get(const Option<T>& opt) {
    if constexpr (I == 0) return opt.unwrap();
}

// 增强CTAD
template<typename T>
Option(Some<T>) -> Option<T>;

Option(_None_t) -> Option<std::monostate>;  // 显式空类型


template<typename T, typename Fn>
constexpr auto operator|(const Option<T>& opt, Fn&& fn){
    return (opt.and_then(std::forward<Fn>(fn)));
}

template<typename T, typename Fn>
constexpr auto operator|(Option<T> && opt, Fn&& fn){
    return (std::move(opt).and_then(std::forward<Fn>(fn)));
}

template<
    typename T, 
    typename Tdecay = std::decay_t<T>
>
[[nodiscard]] Option<Tdecay> optcond(bool cond, T&& value){
    if(cond) return Some<Tdecay>(std::forward<T>(value));
    else return None;
}

template<
    typename T, 
    typename Tdecay = std::decay_t<T>
>
[[nodiscard]] Option<Tdecay> optcond(bool cond, Some<T>&& value){
    if(cond) return Some<Tdecay>(std::forward<Some<T>>(value));
    else return None;
}


template <typename T>
struct __unwrap_helper<Option<T>> {
    using Obj = Option<T>;
    // Unwrap a non-const rvalue Option
    static constexpr T unwrap(Obj && obj) {
        return std::move(obj.unwrap());
    }

    static constexpr T unwrap(const Obj & obj) {
        return obj.value();
    }

    static constexpr void unexpected(Obj && obj) {
        return std::move(obj.unexpected());
    }

    static constexpr void unexpected(const Obj & obj) {
        return obj.unexpected();
    }
};

}


namespace std{
    // 结构化绑定支持 (C++17)
    template<typename T>
    struct tuple_size<ymd::Option<T>> 
        : integral_constant<std::size_t, 1> {};

    template<typename T>
    struct tuple_element<0, ymd::Option<T>> {
        using type = T;
    };

}