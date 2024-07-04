#pragma once

#include "spi.hpp"

class SpiSw: public Spi{
protected:
    volatile int8_t occupied = -1;
    GpioConcept & sclk_gpio;
    GpioConcept & mosi_gpio;
    GpioConcept & miso_gpio;

    uint16_t delays = 100;
    uint8_t data_bits = 8;
    bool m_msb = true;

    __fast_inline void delayDur(){
        delayMicroseconds(delays);
    }

    void clk(){
        delayDur();
        sclk_gpio.set();
        delayDur();
        sclk_gpio.clr();
        delayDur();
    }

    void clk_up(){
        // delayDur();
        sclk_gpio.clr();
        delayDur();
        sclk_gpio.set();
        delayDur();
    }

    void clk_down(){
        delayDur();
        sclk_gpio.clr();
        delayDur();
    }

    void clk_down_then_up(){
        delayDur();
        sclk_gpio.clr();
        delayDur();
        sclk_gpio.set();
        delayDur();
    }

    Error lead(const uint8_t index) override{
        auto ret = Spi::lead(index);
        delayDur();
        return ret;
    }

    void trail() override{
        sclk_gpio.set();
        delayDur();
        Spi::trail();
    }
protected :
public:

    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin):sclk_gpio(_sclk_pin),
                mosi_gpio(_mosi_pin), miso_gpio(_miso_pin){;}
    SpiSw(GpioConcept & _sclk_pin,GpioConcept & _mosi_pin,
            GpioConcept & _miso_pin,GpioConcept & _cs_pin):SpiSw(_sclk_pin, _mosi_pin, _miso_pin){
                bindCsPin(_cs_pin, 0);
            }
    Error write(const uint32_t data) override {
        // delayDur();

        // for(uint16_t mask = m_msb ? 1 << (data_bits - 1) : 0x01; mask; mask = m_msb ? (mask >> 1) : (mask << 1)){
        //     mosi_gpio.write(mask & data);
        //     clk();
        // }
        uint32_t dummy;
        transfer(dummy, data, false);
        return Bus::ErrorType::OK;
    }

    Error read(uint32_t & data, bool toAck = true) {
        // uint8_t ret = 0;
        // delayDur();

        // clk_up();

        // ret |= m_msb ? miso_gpio.read() : miso_gpio.read() << (data_bits - 1);

        // for(uint8_t i = 0; i < data_bits - 1; i++){
        //     clk_down_then_up();
        //     // ret <<= 1;
        //     ret = m_msb ? (ret << 1) : (ret >> 1);
        //     ret |= miso_gpio.read();
        // }

        // clk_down();

        // data = ret;
        uint32_t ret;
        static constexpr uint32_t dummy = 0;
        transfer(ret, dummy, toAck); 
        return Bus::ErrorType::OK;
    }

    Error transfer(uint32_t & data_rx, const uint32_t data_tx, bool toAck = true){
        uint8_t ret = 0;

        sclk_gpio.set();

        // if(m_msb)
        //     mosi_gpio = data_tx & (1 << (data_bits - 1));
        // else
        //     mosi_gpio = data_tx & (0x01);


        // if(m_msb) ret |= miso_gpio.read();
        // else ret |= (miso_gpio.read() << (data_bits - 1)) ;

        for(uint8_t i = 0; i < data_bits; i++){
            if(m_msb){
                mosi_gpio = bool(data_tx & (1 << (data_bits - 2 - i)));
                // clk_down_then_up();
                ret <<= 1; ret |= miso_gpio.read();
                delayDur();
            }else{
                sclk_gpio = true;
                delayDur();
                mosi_gpio = bool(data_tx & (1 << (i)));
                delayDur();
                sclk_gpio = false;
                delayDur();
                // clk_down_then_up();
                ret >>= 1; ret |= (miso_gpio.read() << (data_bits - 1)) ;
                delayDur();
            }
        }

        sclk_gpio.set();

        data_rx = ret;
        return Bus::ErrorType::OK;
    }

    void configBaudRate(const uint32_t baudRate) {
        if(baudRate == 0){
            delays = 0;
        }else{
            uint32_t b = baudRate / 1000;
            delays = 200 / b;
        }
    }

    void init(const uint32_t baudRate){
        configBaudRate(baudRate);
        init();
    }

    void init(){
        mosi_gpio.outpp();
        sclk_gpio.outpp(1);

        for(uint8_t i = 0; i < cs_port.length(); i++){
            if(cs_port.isIndexValid(i)){
                auto & cs_gpio = cs_port[i];
                cs_gpio.outpp(1);
            }
        }

        miso_gpio.inpd();
    }

    void configBitOrder(const bool en) override {
        m_msb = en;
    }
};

