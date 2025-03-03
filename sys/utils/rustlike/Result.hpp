#pragma once

#include "sys/core/platform.h"
#include "util.hpp"
#include <variant>
#include <source_location>

namespace ymd{

template<typename T, typename E>
class Result;

template<typename T = void>
struct Ok{
public:
    using TDecay = std::decay_t<T>;

    // template<typename TDummy>
    constexpr Ok(auto val):val_((val)){}

    template<typename E>
    constexpr operator Result<T, E>() const{
        return Result<T, E>{std::move(val_)};
    }

    constexpr operator T() const{
        return val_;
    }
private:
    TDecay val_;
};

template<>
struct Ok<void>{
public:
    constexpr Ok() = default;

    template<typename E>
    constexpr operator Result<void, E>() const{
        return Result<void, E>{};
    }
};

template<typename E>
struct Err{
public:
    template<typename E2>
    constexpr Err(const Err<E2> & err):val_(std::forward<E>(err.val_)){}
    template<typename E2>
    constexpr Err(Err<E2> && err):val_(std::forward<E>(err.val_)){}

    template<typename E2>
    constexpr Err(const E2 & err):val_(std::forward<E>(err)){}
    template<typename E2>
    constexpr Err(E2 && err):val_(std::forward<E>(err)){}

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

template<typename TDummy = void>
Ok() -> Ok<void>;

template<typename E>
Err(E && val) -> Err<std::decay_t<E>>;


template<typename T, typename E>
struct _Storage{
public:
    using Data = std::variant<T, E>;

    constexpr _Storage(T && val):
        data_(std::forward<T>(val)){;}

    constexpr _Storage(E && val):
        data_(std::forward<E>(val)){;}

    constexpr _Storage(const _Storage &) = default;
    constexpr _Storage(_Storage &&) = default;

    constexpr bool ok() const{return std::holds_alternative<T>(data_);}
    constexpr bool wrong() const{return std::holds_alternative<E>(data_);}

    constexpr T unwrap() const{return std::get<T>(data_);}
    constexpr E error() const{return std::get<E>(data_);}
private:
    Data data_;
};

template<typename E>
struct _Storage<void, E>{
    using Data = std::optional<E>;
    constexpr _Storage(Ok<void> &&):
        data_(std::nullopt){;}

    constexpr _Storage(const Ok<void> &):
        data_(std::nullopt){;}

    // constexpr _Storage(Err<E> && val):
        // data_(std::forward<E>(val)){;}

    constexpr _Storage(const Err<E> & val):
        data_(std::forward<E>(val)){;}

    constexpr _Storage(const _Storage &) = default;
    constexpr _Storage(_Storage &&) = default;

    constexpr bool ok() const{return !data_.has_value();}
    constexpr bool wrong() const{return data_.has_value();}

    constexpr void unwrap() const{}
    constexpr E error() const{return (data_.value());}
private:
    Data data_;
};



template<typename T, typename E>
class Result{
private:
    // using Storage = std::variant<T, E>;


    using Storage = _Storage<T, E>;
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
            return result_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            __PANIC_EXPLICIT_SOURCE(loc, std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    friend class _Loc;
public:
    // template<typename TDummy = void>
    // requires (!std::is_void_v<T>)
    // constexpr Result(T value) : result_(std::move(value)) {}

    // template<typename TDummy = void>
    // requires (std::is_void_v<T>)
    // constexpr Result(void){}

    constexpr Result(E error) : result_(std::move(error)) {}
    

    // template<typename T>
    constexpr Result(Ok<T> && value) : result_((value)){}
    constexpr Result(const Ok<T> & value) : result_((value)){}

    // template<typename E>
    // constexpr Result(Err<E> && error) : result_((error)){}
    constexpr Result(const Err<E> & error) : result_((error)){}

    
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
        return result_.ok();
    }

    constexpr bool wrong() const {
        return result_.wrong();
    }
    
    constexpr T unwrap() const {
        if (likely(ok())) {
            return result_.unwrap();
        } else {
            exit(1);
        }
    }

    template<typename ... Args>
    constexpr T expect(Args && ... args) const{
        if (likely(ok())) {
            return result_.unwrap();
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
            return result_.error();
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