#pragma once

#include "sys/core/platform.h"
#include "util.hpp"
#include <variant>

namespace ymd{


// template<typename T, typename E>
// class Result{
// private:
//     bool is_ok_;
//     union {
//         T value_;
//         E error_;
//     };

// public:
//     // Ok 构造函数
//     Result(const T & value):
//         is_ok_(true), value_(value)
//     {}

//     // Ok 移动构造函数
//     Result(T && value):
//         is_ok_(true), value_(std::move(value))
//     {}

//     // Err 构造函数
//     Result(const E & error):
//         is_ok_(false), error_(error)
//     {}

//     // Err 移动构造函数
//     Result(E && error):
//         is_ok_(false), error_(std::move(error))
//     {}

//     // 检查是否为 Ok
//     bool is_ok() const {
//         return is_ok_;
//     }

//     // 检查是否为 Err
//     bool is_err() const {
//         return !is_ok_;
//     }

//     // 获取 Ok 值
//     const T & ok() const {
//         if (!is_ok_) {
//             PANIC("Called 'ok' on an 'Err' value");
//         }
//         return value_;
//     }

//     // 获取 Err 值
//     const E & err() const {
//         if (is_ok_) {
//             PANIC("Called 'err' on an 'Ok' value");
//         }
//         return error_;
//     }

//     // 如果是 Ok，则返回值；否则返回默认值
//     T unwrap_or(const T & default_value) const {
//         return is_ok_ ? value_ : default_value;
//     }

//     // 如果是 Ok，则返回值；否则调用 PANIC 并传递错误信息
//     T unwrap() const {
//         if (!is_ok_) {
//             PANIC("Called 'unwrap' on an 'Err' value");
//         }
//         return value_;
//     }

//     // 如果是 Ok，则返回值；否则调用 PANIC 并传递自定义错误信息
//     T expect(const std::string & message) const {
//         if (!is_ok_) {
//             PANIC(message);
//         }
//         return value_;
//     }
// };



template<typename Ret, typename Error>
class Result_t {
private:
    std::variant<Ret, Error> result_;

public:
    // 构造函数，用于成功情况
    constexpr Result_t(Ret value) : result_(std::move(value)) {}

    // 构造函数，用于错误情况
    constexpr Result_t(Error error) : result_(std::move(error)) {}

    // 检查是否成功
    constexpr bool ok() const {
        return std::holds_alternative<Ret>(result_);
    }

    // 检查是否出错
    constexpr bool wrong() const {
        return std::holds_alternative<Error>(result_);
    }

    // 获取成功值，如果当前是错误状态则抛出异常
    constexpr Ret unwrap() const {
        if (likely(ok())) {
            return std::get<Ret>(result_);
        } else {
            HALT
        }
    }

    // 获取错误值，如果当前是成功状态则抛出异常
    constexpr Error unwrap_err() const {
        if (likely(wrong())) {
            return std::get<Error>(result_);
        } else {
            HALT
        }
    }

    constexpr operator bool () const {
        return ok();
    }
};

// Specialization for std::optional
template <typename T, typename E>
struct __unwrap_helper<Result_t<T, E>> {
    using Obj = Result_t<T, E>;
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
        return obj.unwrap_err();
    }
};



}