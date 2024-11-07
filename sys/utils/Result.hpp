#pragma once

namespace yumud{


template<typename T, typename E>
class Result{
private:
    bool is_ok_;
    union {
        T value_;
        E error_;
    };

public:
    // Ok 构造函数
    Result(const T & value):
        is_ok_(true), value_(value)
    {}

    // Ok 移动构造函数
    Result(T && value):
        is_ok_(true), value_(std::move(value))
    {}

    // Err 构造函数
    Result(const E & error):
        is_ok_(false), error_(error)
    {}

    // Err 移动构造函数
    Result(E && error):
        is_ok_(false), error_(std::move(error))
    {}

    // 检查是否为 Ok
    bool is_ok() const {
        return is_ok_;
    }

    // 检查是否为 Err
    bool is_err() const {
        return !is_ok_;
    }

    // 获取 Ok 值
    const T & ok() const {
        if (!is_ok_) {
            PANIC("Called 'ok' on an 'Err' value");
        }
        return value_;
    }

    // 获取 Err 值
    const E & err() const {
        if (is_ok_) {
            PANIC("Called 'err' on an 'Ok' value");
        }
        return error_;
    }

    // 如果是 Ok，则返回值；否则返回默认值
    T unwrap_or(const T & default_value) const {
        return is_ok_ ? value_ : default_value;
    }

    // 如果是 Ok，则返回值；否则调用 PANIC 并传递错误信息
    T unwrap() const {
        if (!is_ok_) {
            PANIC("Called 'unwrap' on an 'Err' value");
        }
        return value_;
    }

    // 如果是 Ok，则返回值；否则调用 PANIC 并传递自定义错误信息
    T expect(const std::string & message) const {
        if (!is_ok_) {
            PANIC(message);
        }
        return value_;
    }
};


}