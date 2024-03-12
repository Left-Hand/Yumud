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

    void delayDur(){
        // volatile uint16_t i = delays;
        // while(i--) __nop;
        delayMicroseconds(4);
    }

    void clk(){
        delayDur();
        scl.set();
        delayDur();
        scl.clr();
    }

    void clkr(){
        delayDur();
        scl.clr();
        delayDur();
        scl.set();
        delayDur();
        scl.clr();
        delayDur();
    }

    void clk_up(){
        delayDur();
        scl.clr();
        delayDur();
        scl.set();
        delayDur();
    }

    void clk_down(){
        delayDur();
        scl.clr();
        delayDur();
    }

    void clk_down_then_up(){
        clk_down();
        clk_up();
    }
    void ack(){
        delayDur();
        scl.clr();
        delayDur();
        sda.OutOD();
        sda.clr();
        delayDur();
        scl.set();
        delayDur();
        scl.clr();
        // sda = true;
        delayDur();
    }

    void nack(void) {
        delayDur();
        scl.clr();
        delayDur();
        sda.OutOD();
        sda.set();
        delayDur();
        scl.set();
        delayDur();
        scl.clr();
        sda.set();
        delayDur();
    }

    Error wait_ack(){
        sda.InFloating();
        sda.set();
        delayDur();
        scl.set();
        uint32_t begin_t = micros();
        while(sda.read()){
            if(micros() - begin_t >= timeout){
                delayDur();
                scl.clr();
                return ErrorType::TIMEOUT;
            }
        }
        delayDur();
        scl.clr();
        delayDur();
        return ErrorType::OK;
    }

    Error start(const uint8_t & _address) override{

        scl.OutOD();
        sda.OutOD();
        sda.set();
        scl.set();
        delayDur();
        sda.clr();
        delayDur();
        scl.clr();
        delayDur();
        write(_address);

        return ErrorType::OK;
    }

    void stop() override {
        scl.clr();
        sda.OutOD();
        sda.clr();
        delayDur();
        scl.set();
        delayDur();
        sda.set();
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

        for(uint8_t mask = 0x80; mask; mask >>= 1){
            sda.write(mask & data);
            clk();
        }

        return wait_ack();
    }

    Error read(uint32_t & data, bool toAck = true) {
        uint8_t ret = 0;

        sda.set();
        sda.InPullUP();
        delayDur();

        clk_up();
        delayDur();
        ret |= sda.read();
        for(uint8_t i = 0; i < 7; i++){
            clk_down_then_up();
            ret <<= 1; ret |= sda.read();
        }

        clk_down();

        delayDur();
        sda.clr();
        if(toAck) ack();
        else nack();
        data = ret;
        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t & data_tx, bool toAck){
        return ErrorType::OK;
    }

    void init(const uint32_t & baudRate) override {
        sda.set();
        sda.OutOD();
        scl.set();
        scl.OutOD();
    }
    void configDataSize(const uint8_t & data_size) override {;}
    void configBaudRate(const uint32_t & baudRate) override {;}
    void configBitOrder(const bool & msb) override {;}
};

#endif
