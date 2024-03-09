#ifndef __SERBUS_HPP__

#define __SERBUS_HPP__

#include "bus.hpp"

class BusDrv;

class SerBus:public Bus{
public:
    enum Mode:uint8_t{
        RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
    };

    Mode mode;
protected:
    virtual Error begin_use(const uint8_t & index = 0) = 0;
    virtual void end_use() = 0;
    virtual bool is_idle() = 0;
    virtual bool owned_by(const uint8_t & index = 0) = 0;

    friend BusDrv;

public:

    __fast_inline Error begin(const uint8_t & index = 0) override{
        if(is_idle()){
            return begin_use(index);
            // return ErrorType::OK;
        }else if(owned_by(index)){
            return begin_use(index);
            // return ErrorType::OK;
        }else{
            return ErrorType::OCCUPIED;
        }
    }

    __fast_inline void end() override {
        end_use();
    }

    virtual void configBitOrder(const bool & msb) = 0;
};

// class BusFullDuplex:public Bus{
// };

// class BusHalfDuplex:public Bus{
// public:
//     void transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck = true) override = delete;
// };

// class BusROSimplex:public BusHalfDuplex{
// public:
//     Error write(const uint32_t & data) override = delete;
// };

// class BusWOSimplex:public BusHalfDuplex{
// public:

// };
#endif