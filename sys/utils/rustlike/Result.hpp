#pragma once

#include "sys/core/platform.h"
#include "util.hpp"
#include <variant>

namespace ymd{

template<typename Ret>
struct Ok{
    Ret val;
};

template<typename Error>
struct Err{
    Error val;
};

template<typename T, typename Tdecay = std::decay_t<T>>
Ok(T && val) -> Ok<Tdecay>;

template<typename E, typename Edecay = std::decay_t<E>>
Err(E && val) -> Err<Edecay>;

template<typename Ret, typename Error>
class Result{
private:
    std::variant<Ret, Error> result_;
public:

    constexpr Result(Ret value) : result_(std::move(value)) {}

    constexpr Result(Error error) : result_(std::move(error)) {}
    

    constexpr Result(Ok<Ret> value) : result_(std::move(value.val)) {}

    constexpr Result(Err<Error> error) : result_(std::move(error.val)) {}

    
    // 映射成功值
    template<typename F>
    auto map(F&& fn) -> Result<std::invoke_result_t<F, Ret>, Error> {
        if (ok()) return fn(unwrap());
        else return err();
    }

    // 链式处理
    template<typename F>
    auto then(F&& fn) -> Result<std::invoke_result_t<F, Ret>, Error> {
        if (ok()) return fn(unwrap());
        else return err();
    }
    
    constexpr bool ok() const {
        return std::holds_alternative<Ret>(result_);
    }

    constexpr bool wrong() const {
        return std::holds_alternative<Error>(result_);
    }
    
    constexpr Ret unwrap() const {
        if (likely(ok())) {
            return std::get<Ret>(result_);
        } else {
            exit(1);
        }
    }

    constexpr Error err() const {
        if (likely(wrong())) {
            return std::get<Error>(result_);
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