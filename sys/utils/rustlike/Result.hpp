#pragma once

#include "sys/core/platform.h"
#include "sys/debug/debug.hpp"
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

    constexpr Ok(auto val):val_((val)){}

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
};

template<typename E>
struct Err{
public:
    template<typename E2>
    constexpr Err(const Err<E2> & err):val_(std::forward<E>(err.val_)){}
    template<typename E2>
    constexpr Err(Err<E2> && err):val_(std::forward<E>(err.val_)){}

    template<typename E2>
    constexpr Err(const E2 & err):val_(err){}
    template<typename E2>
    constexpr Err(E2 && err):val_(std::forward<E>(err)){}
    constexpr operator E() const{
        return val_;
    }
private:
    E val_;
};


template<>
struct Err<void>{
public:
    constexpr Err() = default;
};


template<typename T>
Ok(T && val) -> Ok<std::decay_t<T>>;

template<typename TDummy = void>
Ok() -> Ok<void>;

template<typename E>
Err(E && val) -> Err<std::decay_t<E>>;

template<typename TDummy = void>
Err() -> Err<void>;

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

    constexpr bool is_ok() const{return std::holds_alternative<T>(data_);}
    constexpr bool is_err() const{return std::holds_alternative<E>(data_);}

    constexpr T unwrap() const{return std::get<T>(data_);}
    constexpr E unwrap_err() const{return std::get<E>(data_);}
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

    constexpr _Storage(const Err<E> & val):
        data_(std::forward<E>(val)){;}

    constexpr _Storage(const _Storage &) = default;
    constexpr _Storage(_Storage &&) = default;

    constexpr bool is_ok() const{return !data_.has_value();}
    constexpr bool is_err() const{return data_.has_value();}

    constexpr void unwrap() const{}
    constexpr E unwrap_err() const{return (data_.value());}
private:
    Data data_;
};

template<>
struct _Storage<void, void>{
    constexpr _Storage(Ok<void> &&){}

    constexpr _Storage(const Ok<void> &){}

    constexpr _Storage(const Err<void> & val){}

    constexpr _Storage(const _Storage &) = default;
    constexpr _Storage(_Storage &&) = default;

    constexpr bool is_ok() const{return is_ok_;}
    constexpr bool is_err() const{return !is_ok_;}
    constexpr void unwrap() const{}
    constexpr void unwrap_err() const{}
private:
    bool is_ok_;
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
        if (likely(is_ok())) {
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

    constexpr Result(E unwrap_err) : result_(std::move(unwrap_err)) {}
    

    // template<typename T>
    constexpr Result(Ok<T> && value) : result_((value)){}
    constexpr Result(const Ok<T> & value) : result_((value)){}

    // template<typename E>
    // constexpr Result(Err<E> && unwrap_err) : result_((unwrap_err)){}
    constexpr Result(const Err<E> & unwrap_err) : result_((unwrap_err)){}

    
    // 映射成功值
    template<typename F>
    constexpr auto map(F&& fn) -> Result<std::invoke_result_t<F, T>, E> const {
        if (is_ok()) return fn(unwrap());
        else return unwrap_err();
    }

    // 链式处理
    template<typename F>
    constexpr auto and_then(F&& fn) -> Result<std::invoke_result_t<F, T>, E> const {
        if (is_ok()) return fn(unwrap());
        else return unwrap_err();
    }

    template<typename F>
    constexpr void if_ok(F&& fn) const {
        if (is_ok()) {
            (fn)();
        }
    }


    constexpr bool is_ok() const {
        return result_.is_ok();
    }

    constexpr bool is_err() const {
        return result_.is_err();
    }
    
    constexpr T unwrap() const {
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            exit(1);
        }
    }

    template<typename ... Args>
    constexpr T expect(Args && ... args) const{
        if (likely(is_ok())) {
            return result_.unwrap();
        } else {
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTS(std::forward<Args>(args)...);
            #endif
            exit(1);
        }
    }
    
    template<typename ... Args>
    const Result & check(Args && ... args) const{
        if(unlikely(is_err())){
            #ifdef __DEBUG_INCLUDED
            DEBUG_PRINTS(unwrap_err(), std::forward<Args>(args)...);
            #endif
            exit(1);
        }
        return *this;
    } 
    constexpr _Loc loc(const std::source_location & loca = std::source_location::current()) const{
        return {*this, loca};
    }

    constexpr E unwrap_err() const {
        if (likely(is_err())) {
            return result_.unwrap_err();
        } else {
            exit(1);
        }
    }

};

template<typename E>
Result(Err<E> && val) -> Result<void, E>;

template<typename TDummy>
Result() -> Result<void, void>;

// Specialization for std::optional
template <typename T, typename E>
struct __unwrap_helper<Result<T, E>> {
    using Obj = Result<T, E>;
    // Unwrap a non-const rvalue optional
    static constexpr bool is_ok(const Obj & obj) {
        return obj.is_ok();
    }
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
        return obj.unwrap_err();
    }
};




}