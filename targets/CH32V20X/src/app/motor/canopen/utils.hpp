#pragma once

#include "hal/bus/can/can.hpp"
#include <variant>
#include <unordered_map>
#include <tuple>
#include "sys/string/String.hpp"

namespace ymd::canopen{

using Can = hal::Can;
using CanMsg = hal::CanMsg;

class CanDriver {
public:
    // using Can = hal::Can;
    // using CanMsg = hal::CanMsg;

    virtual ~CanDriver() = default;
    virtual bool write(const CanMsg & frame) = 0;
    virtual bool read(CanMsg & frame) = 0;
};


template<typename T>
class optref{
private:
    T * ptr_;

    void check() const{
        if(ptr_ == nullptr){
            HALT;
        }
    }
public:
    optref(T & ptr): ptr_(&ptr){}
    optref(T * ptr): ptr_(ptr){}
    optref(std::nullopt_t): ptr_(nullptr){}

    bool has_value() const{return ptr_ != nullptr;}

    T & value() const{check();return *ptr_;}
    T & unwarp() const{check();return *ptr_;}
};

class SubEntry;

class CanOpenListener {
public:
    virtual void onObjDictChange(SubEntry & subEntry) = 0;
    virtual void onMessage(const CanMsg & msg) = 0;
    virtual ~CanOpenListener() = default;
};

template<typename T>
struct E_Item {
    T v_;

    constexpr E_Item(T v) : v_(v) {}

    // 添加比较操作符，方便枚举值的比较
    bool operator==(const E_Item& other) const { return v_ == other.v_; }
    bool operator!=(const E_Item& other) const { return v_ != other.v_; }
    bool operator<=(const E_Item& other) const { return v_ <= other.v_; }
};


}