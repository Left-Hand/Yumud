#pragma once

#include "util.hpp"

namespace ymd{

struct _None_t{
};

static inline _None_t None = {};

template<typename T>
struct Some{
public:
    constexpr Some(const T & val):val_(val){}
    constexpr Some(T && val):val_(std::move(val)){;}

    constexpr T & operator*(){
        return val_;
    }
private:
    T val_;
};

template<>
struct Some<void>{
public:
};

//CTAD
template<typename T>
Some(T && val) -> Some<std::decay_t<T>>;

template<typename TDummy = void>
Some() -> Some<void>;




template<typename T>
class Option{
private:
    bool exists_;
    T value_;
public:
    // Option(std::nullopt_t):
    //     exists_(false)
    // {}

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

    // T * operator->() {
    //    return &unwarp();
    // }

    // const T * operator->() const {
    //     return &unwarp();
    // }

    // T && operator*() && {
    //     return std::move(unwarp());
    // }
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