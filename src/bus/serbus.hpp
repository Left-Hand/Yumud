#ifndef __SERBUS_HPP__

#define __SERBUS_HPP__

#include "bus.hpp"

class BusDrv;

class SerBus:public Bus{
protected:
    virtual void begin_use(const uint8_t & index = 0) = 0;
    virtual void end_use() = 0;
    virtual bool is_idle() = 0;
    virtual bool owned_by(const uint8_t & index = 0) = 0;

    friend BusDrv;
public:

    __fast_inline Error begin(const uint8_t & index = 0) override {
        if(is_idle()){
            begin_use(index);
            return ErrorType::OK;
        }else{
            return (owned_by(index)) ? ErrorType::OK : ErrorType::OCCUPIED;
        }
    }

    __fast_inline void end() override {
        end_use();
    }


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