#ifndef __I2C1_HPP__
#define __I2C1_HPP__

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"

class I2c1: public I2cHw{
private:
    volatile int8_t occupied = -1;

    Error start(const uint8_t & _address) override;
    void stop();
protected:

    Error begin_use(const uint8_t & index = 0) override {
        occupied = index >> 1;
        return start(index);
    }

    void reset() override;

    void end_use() override {
        stop();
        occupied = -1;
    }

    bool is_idle() override {
        return (occupied >= 0 ? false : true);
    }

    bool owned_by(const uint8_t & index = 0) override{
        return (occupied == (index >> 1));
    }

    void lock_avoid() override;

public:

    I2c1(){;}

    Error write(const uint32_t & data) override ;
    Error read(uint32_t & data, bool toAck = true) override;

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override ;
    void enableHwTimeout(const bool en = true) override;
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};


#if defined(HAVE_I2C1)
extern I2c1 i2c1;
#endif

#endif
