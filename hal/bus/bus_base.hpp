#pragma once

#include "sys/core/platform.h"
#include "BusTrait.hpp"
#include "sys/stream/stream.hpp"
#include "bus_enums.hpp"

namespace ymd{

class Bus:public BusTrait{
public:
    enum class ErrorType:uint8_t{
        OK,
        ALREADY,
        OCCUPIED,
        TIMEOUT,
        OVERLOAD,
        NO_ACK,
        NO_CS_PIN,
        ZERO_LENGTH,
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
        bool operator !=(const ErrorType & _type){return type != _type;}
        explicit operator bool() {return type != ErrorType::OK;}

        bool ok() const {return type == ErrorType::OK;}
        explicit operator ErrorType() {return type;}
    };
    
    // scexpr int a = sizeof(Error);

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
        DELETE_COPY_AND_MOVE(Locker)

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