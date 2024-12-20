#pragma once

#include "sys/debug/debug_inc.h"


namespace ymd{

template<typename T>
class Optional{
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

    T * operator->() {
       return &unwarp();
    }

    const T * operator->() const {
        return &unwarp();
    }

    T && operator*() && {
        return std::move(unwarp());
    }
    const T & expect(const char * msg) const {
        if(exists_ == false){
            PANIC(msg);
        }
        return value_;
    }
};


}