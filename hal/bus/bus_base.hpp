#pragma once

#include "sys/core/platform.h"
#include "BusTrait.hpp"
#include "sys/stream/stream.hpp"
#include "bus_enums.hpp"

namespace ymd{

struct BusError{
public:
    enum ErrorType:uint8_t{
        OK,
        ALREADY,
        OCCUPIED,
        TIMEOUT,
        OVERLOAD,
        NO_ACK,
        NO_CS_PIN,
        ZERO_LENGTH,
    };

    ErrorType type = ErrorType::OK;

    BusError(const ErrorType & _type):type(_type){;}
    BusError(ErrorType && _type):type(_type){;}
    BusError(const BusError & other):type(other.type){;}
    BusError(BusError && other):type(other.type){;}
    BusError & operator = (const BusError & other){type = other.type; return *this;}
    BusError & operator = (BusError && other){type = other.type; return *this;}

    bool operator ==(const ErrorType & _type){return type == _type;}
    bool operator !=(const ErrorType & _type){return type != _type;}
    explicit operator bool() {return type != ErrorType::OK;}

    bool ok() const {return type == ErrorType::OK;}
    explicit operator ErrorType() {return type;}
};


class Bus:public BusTrait{
public:
    using Error = BusError;
    using Mode = CommMode;


protected:
    Mode mode = Mode::TxRx;

private:
    class Locker{
    public:
        Locker * last_ = nullptr;
        Locker * next_ = nullptr;
    protected:
        uint16_t req:8 = 0;
        uint16_t oninterrupt_:1 = false;
        uint16_t locked_:1 = false;
    public:
        Locker(const Locker & other) = delete;
        Locker(Locker && other) = delete;
        Locker(){;}
        Locker(Locker * last, Locker * next):last_(last), next_(next){;}

        ~Locker(){
            unlock();
            last_ = next_;
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

    Locker __own_locker__;
    Locker * locker = nullptr;

    virtual Error lead(const uint8_t _address) = 0;
    virtual void trail() = 0;

    void lock(const uint8_t index);
    void unlock();
    bool locked();
    bool owned_by(const uint8_t index = 0);

public:
    Bus():locker(&__own_locker__){;}
    DELETE_COPY_AND_MOVE(Bus)

    Locker createLocker(){
        return Locker{locker, locker};
    }

    Error begin(const uint8_t index){
        if(false == locked()){
            lock(index);
            return lead(index);
        }

        else if(owned_by(index)){
            lock(index);
            return lead(index);
        }else{
            return Error::OCCUPIED;
        }
    }


    void end(){
        trail();
        unlock();
    }

    bool occupied(){
        return locker->locked();
    }
};



OutputStream & operator << (OutputStream & os, const BusError & err);

OutputStream & operator << (OutputStream & os, const BusError::ErrorType & err);

};