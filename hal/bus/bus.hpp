#pragma once

#include "sys/core/system.hpp"
#include "sys/stream/stream.hpp"

namespace yumud{
enum class CommMethod:uint8_t{
    None = 0,
    Blocking,
    Interrupt,
    Dma,
};

enum class CommMode:uint8_t{
    RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
};


class Bus{
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
        ErrorType errorType = ErrorType::OK;

        Error(const ErrorType & _errorType):errorType(_errorType){;}

        bool operator ==(const ErrorType & _errorType){return errorType == _errorType;}
        explicit operator bool() {return errorType != ErrorType::OK;}
        explicit operator ErrorType() {return errorType;}
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
        void lock(const uint8_t index){
            Sys::Exception::disableInterrupt();
            oninterrupt_ = Sys::Exception::isIntrruptActing();
            req = index >> 1;
            locked_ = true;
            Sys::Exception::enableInterrupt();
        }

        void unlock(){
            locked_ = false;
        }

        bool owned_by(const uint8_t index = 0) const {
            return (req == index >> 1) and (Sys::Exception::isIntrruptActing() == oninterrupt_);
        }

        bool locked() const {
            return locked_;
        }
    };

    Lock __m_lock__;
    Lock * locker = nullptr;

    virtual Error lead(const uint8_t _address) = 0;
    virtual void trail() = 0;

    void lock(const uint8_t index){
        if(locker == nullptr) CREATE_FAULT;
        locker->lock(index);
    }

    void unlock(){
        if(locker == nullptr) CREATE_FAULT;
        locker->unlock();
    }

    bool locked(){
        if(locker == nullptr) CREATE_FAULT;
        return locker->locked();
    }

    bool owned_by(const uint8_t index = 0){
        if(locker == nullptr) CREATE_FAULT;
        return locker->owned_by(index);
    }



public:
    Bus():locker(&__m_lock__){;}
    virtual void setBitOrder(const Endian endian){};
    virtual void setDataBits(const uint8_t data_size){};
    virtual void setBaudRate(const uint32_t baudRate) = 0;

    Error begin(const uint8_t index){
        if(locked()){
            lock(index);
            return lead(index);
        }

        else if(owned_by(index)){
            lock(index);
            return lead(index);
        }
        else{
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


class ReadableBus:virtual public Bus{
protected:
public:
    CommMethod rxMethod = CommMethod::None;
    virtual Error read(uint32_t & data, bool toAck = true) = 0;
};

class WritableBus:virtual public Bus{
protected:
public:
    CommMethod txMethod = CommMethod::None;
    virtual Error write(const uint32_t data) = 0;
};

class DuplexBus:public ReadableBus, WritableBus{
public:
// public:
    using WritableBus::txMethod;
    using ReadableBus::rxMethod;
    using WritableBus::write;
    using ReadableBus::read;
    DuplexBus():ReadableBus(), WritableBus(){;}
};

class FullDuplexBus:public DuplexBus{
public:
    virtual Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true) = 0;
};


class ProtocolBus:public DuplexBus{
protected:

};

template<typename Packet>
class PackedBus:public Bus{
private:
    using Bus::setDataBits;//disable this;
public:
    virtual bool write(const Packet & msg) = 0;
    virtual const Packet & read() = 0;
};


}