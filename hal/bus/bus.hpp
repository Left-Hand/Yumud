
#ifndef __BUS_HPP__
#define __BUS_HPP__

#include "../sys/core/platform.h"
#include "../sys/kernel/clock.h"

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
        NACK,
        NO_CS_PIN
    };

    enum Mode:uint8_t{
        RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
    };

    struct Error{
        ErrorType errorType = ErrorType::OK;

        Error(const ErrorType & _errorType):errorType(_errorType){;}

        bool operator ==(const ErrorType & _errorType){return errorType == _errorType;}
        explicit operator bool() {return errorType != ErrorType::OK;}
        explicit operator ErrorType() {return errorType;}
    };



protected:
    Mode mode = TxRx;

private:
    int8_t m_lock = -1;
    int8_t * locker = nullptr;

    virtual Error lead(const uint8_t _address) = 0;
    virtual void trail() = 0;

    void lock(const uint8_t index){
        if(locker == nullptr) exit(1);
        *locker = index >> 1;
    }

    void unlock(){
        if(locker == nullptr) exit(1);
        *locker = -1;
    }

    int8_t wholock(){
        if(locker == nullptr) exit(1);
        return *locker;
    }

    bool is_idle(){
        if(locker == nullptr) exit(1);
        return (*locker >= 0 ? false : true);
    }

    bool owned_by(const uint8_t index = 0){
        if(locker == nullptr) exit(1);
        return (*locker == index >> 1);
    }



public:
    Bus():locker(&m_lock){;}
    virtual void configBitOrder(const Endian endian){};
    virtual void configDatabits(const uint8_t data_size){};
    virtual void configBaudRate(const uint32_t baudRate) = 0;

    Error begin(const uint8_t index){
        if(is_idle()){
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
    using Bus::configDatabits;//disable this;
public:
    virtual bool write(const Packet & msg) = 0;
    virtual const Packet & read() = 0;
};


#endif // !__PRINTABLE_HPP__