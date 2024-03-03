#ifndef I2C_SOFT_H_
#define I2C_SOFT_H_

#include "i2c.hpp"
#include "../../gpio/gpio.hpp"
#include "../uart/uart2.hpp"

class I2cSw: public I2c{
private:
    volatile int8_t occupied = -1;
    GpioBase & scl;
    GpioBase & sda;

    uint16_t delays = 100;

    __fast_inline volatile void delayDur(){
        // __nopn(delays);
        // volatile uint16_t i = delays;
        // while(i--);
        delayMicroseconds(3);
    }

    void clk(){
        delayDur();
        scl = true;
        delayDur();
        scl = false;
    }

    void clkr(){
        delayDur();
        scl = false;
        delayDur();
        scl = true;
        delayDur();
        scl = false;
        delayDur();
    }

    void clk_up(){
        delayDur();
        scl = false;
        delayDur();
        scl = true;
        delayDur();
    }

    void clk_down(){
        scl = false;
    }

    void clk_down_then_up(){
        clk_down();
        clk_up();
    }
    void ack(){
        delayDur();
        scl = false;
        delayDur();
        sda.OutOD();
        sda = false;
        delayDur();
        delayDur();
        scl = true;
        delayDur();
        delayDur();
        scl = false;
        // sda = true;
        delayDur();
        delayDur();
    }

    void nack(void) {
        delayDur();
        scl = false;
        delayDur();
        sda.OutOD();
        sda = true;
        delayDur();
        delayDur();
        scl = true;
        delayDur();
        delayDur();
        scl = false;
        // sda = true;
        delayDur();
        delayDur();
    }

    bool wait_ack(){
        bool ret;
        sda.InFloating();
        sda = true;
        delayDur();
        scl = true;
        delayDur();
        ret = sda.read();
        scl = false;
        delayDur();

        return ret;
    }

    Error start(const uint8_t & _address) override{

        scl.OutOD();
        sda.OutOD();
        sda = true;
        scl = true;
        delayDur();
        sda = false;
        delayDur();
        scl = false;
        delayDur();
        write(_address);

        return ErrorType::OK;
    }

    void stop() override {
        scl = false;
        sda.OutOD();
        sda = false;
        delayDur();
        scl = true;
        delayDur();
        sda = true;
        delayDur();
    }


protected :
     Error begin_use(const uint8_t & index = 0) override {
        occupied = index >> 1;
        return start(index);
    }
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

    void reset() override {};
    void lock_avoid() override {};
public:

    I2cSw(GpioBase & _scl,GpioBase & _sda, const uint16_t & _delays = 10):scl(_scl), sda(_sda), delays(_delays){;}

    Error write(const uint32_t & data) override {
        sda.OutOD();
        delayDur();
        sda.write(0x80 & data);
        clk();
        sda.write(0x40 & data);
        clk();
        sda.write(0x20 & data);
        clk();
        sda.write(0x10 & data);
        clk();
        sda.write(0x08 & data);
        clk();
        sda.write(0x04 & data);
        clk();
        sda.write(0x02 & data);
        clk();
        sda.write(0x01 & data);
        clk();

        wait_ack();

        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;
        sda.InFloating();
        sda = true;
        delayDur();
        
        clk_up();
        ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();

        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down_then_up();
        ret <<= 1; ret |= sda.read();
        clk_down();

        sda = false;
        if(toAck) ack();
        else nack();
        data = ret;
        // sda.OutOD();

        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        write(data_tx);
        read(data_rx, toAck);
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override {;}
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};

#endif
