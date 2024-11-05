#pragma once

#include "sys/core/platform.h"
#include "BusTrait.hpp"
#include "sys/stream/stream.hpp"
#include "bus_enums.hpp"

namespace yumud{

class Bus:public BusTrait{
public:
    enum class ErrorType{
        OK,
        ALREADY,
        OCCUPIED,
        TIMEOUT,
        OVERLOAD,
        NO_ACK,
        NO_CS_PIN
    };

    struct Error{
        ErrorType type = ErrorType::OK;

        Error(const ErrorType & _type):type(_type){;}
        Error(ErrorType && _type):type(_type){;}
        Error(const Error & other):type(other.type){;}
        Error(Error && other):type(other.type){;}
        Error & operator = (const Error & other){type = other.type; return *this;}
        Error & operator = (Error && other){type = other.type; return *this;}

        bool operator ==(const ErrorType & _type){return type == _type;}
        explicit operator bool() {return type != ErrorType::OK;}
        explicit operator ErrorType() {return type;}
    };

    using Mode = CommMode;


protected:
    Mode mode = Mode::TxRx;

private:
    class Lock{
    protected:
        uint16_t req:8 = 0;
        uint16_t oninterrupt_:1 = false;
        uint16_t locked_:1 = false;
    public:
        void lock(const uint8_t index);

        void unlock(){
            locked_ = false;
        }

        bool owned_by(const uint8_t index) const;

        bool locked() const {
            return locked_;
        }
    };

    Lock __m_lock__;
    Lock * locker = nullptr;

    virtual Error lead(const uint8_t _address) = 0;
    virtual void trail() = 0;

    void lock(const uint8_t index);
    void unlock();
    bool locked();
    bool owned_by(const uint8_t index = 0);

public:
    Bus():locker(&__m_lock__){;}

    Error begin(const uint8_t index){
        if(false == locked()){
            lock(index);
            return lead(index);
        }

        else if(owned_by(index)){
            lock(index);
            return lead(index);
        }else{
            return ErrorType::OCCUPIED;
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



OutputStream & operator << (OutputStream & os, const Bus::ErrorType & err);

OutputStream & operator << (OutputStream & os, const Bus::Error & err);

};