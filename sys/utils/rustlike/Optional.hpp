#pragma once

#include "util.hpp"

namespace ymd{

template<typename T, typename E = std::nullopt_t>
class Optional_t{
private:
    bool exists_;
    T value_;
public:
    Optional():
        exists_(false)
    {}

    Optional(const T & value):
        exists_(true),
        value_(value){}

    Optional(T && value):
        exists_(true),
        value_(std::move(value)){}

    bool has_value() const{ return exists_; }

    const T & value_or(const T & default_value) const{
        return exists_ ? value_ : default_value;
    }

    const T & unwarp() const {
        if(exists_ == false){
            PANIC();
        }
        return value_;
    }


    E unexpected() const {
        return {}
    }

    T * operator->() {
       return &unwarp();
    }

    const T * operator->() const {
        return &unwarp();
    }

    T && operator*() && {
        return std::move(unwarp());
    }
};


// Specialization for std::optional
template <typename T, typename E>
struct __unwrap_helper<Optional_t<T, E>> {
    using Obj = Optional_t<T, E>;
    // Unwrap a non-const rvalue optional
    static constexpr T unwrap(Obj && obj) {
        return std::move(obj.unwrap());
    }

    static constexpr T unwrap(const Obj & obj) {
        return obj.value();
    }

    static constexpr E unexpected(Obj && obj) {
        return std::move(obj.unexpected());
    }

    static constexpr E unexpected(const Obj & obj) {
        return obj.unexpected();
    }
};




}