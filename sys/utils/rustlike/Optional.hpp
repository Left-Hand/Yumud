#pragma once

#include "sys/core/platform.h"
#include "sys/debug/debug.hpp"
#include "util.hpp"
#include <variant>
#include <source_location>
#include "util.hpp"

namespace ymd{


template<typename T>
static constexpr bool is_option_v = false;

template<typename T>
static constexpr bool is_option_v<Option<T>> = true;

template<typename T>
class Option{
private:
    bool exists_;
    T value_;
public:
    Option(_None_t):
        exists_(false)
    {}

    constexpr Option(const Some<T> & value):
        exists_(true),
        value_(value){}

    constexpr Option(Some<T> && value):
        exists_(true),
        value_(std::move(*value)){}

    constexpr bool is_some() const{ return exists_; }
    constexpr bool is_none() const{ return !exists_; }

    constexpr const T & value_or(const T & default_value) const{
        return exists_ ? value_ : default_value;
    }

    constexpr const T & unwarp(const std::source_location & loc = std::source_location::current()) const {
        if(exists_ == false){
            __PANIC_EXPLICIT_SOURCE(loc);
        }
        return value_;
    }

    constexpr void unexpected() const {
        return;
    }

    // // 管道运算符支持 (|> 类似F#)
    // template<typename U, typename Fn>
    // auto operator|(const Option<U>& opt, Fn&& fn) {
    //     return fn(opt);
    // }

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
    auto match(FnSome&& some_fn, FnNone&& none_fn) const& {
        if (is_some()) {
            return std::invoke(std::forward<FnSome>(some_fn), value_);
        } else {
            return std::invoke(std::forward<FnNone>(none_fn));
        }
    }

    // 函数式映射 (Monadic map)
    template<typename Fn>
    auto map(Fn&& fn) const& -> Option<std::invoke_result_t<Fn, const T&>> {
        // using RetType = std::invoke_result_t<Fn, const T&>;
        if (is_some()) {
            return Some(fn(value_));
        }
        return None;
    }

    // 链式操作 (Monadic and_then)
    template<typename Fn>
    auto and_then(Fn&& fn) const& {
        using RetType = std::invoke_result_t<Fn, const T&>;
        static_assert(is_option_v<RetType>, 
            "Fn must return an Option type");
        
        return is_some() ? fn(value_) : RetType(None);
    }
};


// Specialization for std::Option
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