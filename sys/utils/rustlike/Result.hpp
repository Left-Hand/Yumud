#pragma once

#include "sys/core/platform.h"
#include "util.hpp"
#include <variant>
#include <source_location>

namespace ymd{

template<typename T, typename E>
class Result;

template<typename T>
struct Ok{
public:
    constexpr Ok(auto && val):val_(std::forward<T>(val)){}

    template<typename E>
    constexpr operator Result<T, E>() const{
        return Result<T, E>{std::move(val_)};
    }

    constexpr operator T() const{
        return val_;
    }
private:
    T val_;
};

template<typename E>
struct Err{
public:
    constexpr Err(auto && val):val_(std::forward<E>(val)){}

    template<typename T>
    constexpr operator Result<T, E>() const{
        return Result<T, E>{std::move(val_)};
    }

    constexpr operator E() const{
        return val_;
    }
private:
    E val_;
};

// template<typename Ret>
// struct Ok{
//     Ret val;
// };

// template<typename Error>
// struct Err{
//     Error val;
// };


template<typename T>
Ok(T && val) -> Ok<std::decay_t<T>>;

template<typename E>
Err(E && val) -> Err<std::decay_t<E>>;

template<typename T, typename E>
class Result{
private:
    using Storage = std::variant<T, E>;

    Storage result_;

    struct _Loc{
        const Result<T, E> & owner_;
        std::source_location loc_;

        template<typename ... Args>
        constexpr T expect(Args && ... args) const{
            return owner_.except_impl(loc_, std::forward<Args>(args)...);
        }
    };

    template<typename ... Args>
    constexpr T except_impl(
        const std::source_location & loc,
        Args && ... args
    ) const {
        if (likely(ok())) {
            return std::get<T>(result_);
        } else {
            #ifdef __DEBUG_INCLUDED
            __PANIC_EXPLICIT_SOURCE(loc, std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    friend class _Loc;
public:

    constexpr Result(T value) : result_(std::move(value)) {}

    constexpr Result(E error) : result_(std::move(error)) {}
    

    // template<typename T>
    constexpr Result(Ok<T> value) : result_(T(value)){}

    // template<typename E>`
    constexpr Result(Err<E> error) : Result(E(error)){}

    
    // 映射成功值
    template<typename F>
    constexpr auto map(F&& fn) -> Result<std::invoke_result_t<F, T>, E> const {
        if (ok()) return fn(unwrap());
        else return err();
    }

    // 链式处理
    template<typename F>
    constexpr auto then(F&& fn) -> Result<std::invoke_result_t<F, T>, E> {
        if (ok()) return fn(unwrap());
        else return err();
    }
    

    constexpr bool ok() const {
        return std::holds_alternative<T>(result_);
    }

    constexpr bool wrong() const {
        return std::holds_alternative<E>(result_);
    }
    
    constexpr T unwrap() const {
        if (likely(ok())) {
            return std::get<T>(result_);
        } else {
            exit(1);
        }
    }

    template<typename ... Args>
    constexpr T expect(Args && ... args) const{
        if (likely(ok())) {
            return std::get<T>(result_);
        } else {
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTLN(std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }

    constexpr _Loc loc(const std::source_location & loca = std::source_location::current()) const{
        return {*this, loca};
    }

    constexpr E err() const {
        if (likely(wrong())) {
            return std::get<E>(result_);
        } else {
            exit(1);
        }
    }
    
    constexpr operator bool () const {
        return ok();
    }
};

// Specialization for std::optional
template <typename T, typename E>
struct __unwrap_helper<Result<T, E>> {
    using Obj = Result<T, E>;
    // Unwrap a non-const rvalue optional
    static constexpr T && unwrap(Obj && obj) {
        return std::move(obj.unwrap());
    }
    
    static constexpr T unwrap(const Obj & obj) {
        return obj.unwrap();
    }
    
    static constexpr E unexpected(Obj && obj) {
        return std::move(obj.unwrap_err());
    }

    static constexpr E unexpected(const Obj & obj) {
        return obj.err();
    }
};




}