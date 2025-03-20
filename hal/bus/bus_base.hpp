#pragma once

#include "core/platform.h"

#include "BusTrait.hpp"
#include "bus_enums.hpp"

namespace ymd{


struct BusError{
public:
    enum Kind:uint8_t{
        OK,
        ALREADY,
        OCCUPIED,
        TIMEOUT,
        OVERLOAD,
        NO_ACK,
        NO_CS_PIN,
        ZERO_LENGTH,
        VERIFY_FAILD,

        LengthOverflow,

        UNSPECIFIED = 0xff
    };

    Kind type = Kind::OK;

    BusError(const Kind & _type):type(_type){;}
    BusError(Kind && _type):type(_type){;}
    BusError(const BusError & other):type(other.type){;}
    BusError(BusError && other):type(other.type){;}
    BusError & operator = (const BusError & other) = default;
    BusError & operator = (BusError && other) = default;
    __fast_inline BusError & emplace(const BusError & other){type = other.type; return *this;}
    __fast_inline BusError & emplace(BusError && other){type = other.type; return *this;}

    bool operator ==(const Kind & _type){return type == _type;}
    bool operator !=(const Kind & _type){return type != _type;}

    __fast_inline bool wrong() const {return unlikely(type != Kind::OK);}
    __fast_inline bool ok() const {return likely(type == Kind::OK);}

    auto unwrap() const {return type;}

    // template<typename F>
    // auto map(F&& fn) -> BusError<std::invoke_result_t<F, T>, E> {
    //     if (ok()) return fn(unwrap());
    //     else return err();
    // }

    // 链式处理
    template<typename Fn>
    BusError then(Fn && fn){
        if (ok()) return std::forward<Fn>(fn)();
        return *this;
    }

    explicit operator Kind() {return type;}
};


class BusBase{
private:
    class Locker{
    protected:
        uint16_t req:8 = 0;
        uint16_t oninterrupt_:1 = false;
        uint16_t locked_:1 = false;
    public:
        Locker(const Locker & other) = delete;
        Locker(Locker && other) = delete;
        Locker(){;}
        // Locker(Locker * last, Locker * next):last_(last), next_(next){;}

        ~Locker(){
            unlock();
            // last_ = next_;
        }

        void lock(const uint8_t index);

        void unlock(){
            locked_ = false;
        }

        bool owned_by(const uint8_t index) const;

        bool locked() const {
            return locked_;
        }
    };

    Locker __own_locker__ = {};
    Locker & locker;

    virtual BusError lead(const uint8_t _address) = 0;
    virtual void trail() = 0;
public:
    BusBase():locker(__own_locker__){;}

    virtual ~BusBase(){;}
    
    BusBase(const BusBase &) = delete;
    BusBase(BusBase &&) = delete;

    BusError begin(const uint8_t index);

    BusError end();

    bool occupied(){return locker.locked();}
};



OutputStream & operator << (OutputStream & os, const BusError & err);

OutputStream & operator << (OutputStream & os, const BusError::Kind & err);

};