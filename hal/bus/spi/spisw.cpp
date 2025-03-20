#include "spisw.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiSw::init(const uint32_t baudrate, const CommStrategy tx_strategy , const CommStrategy rx_strategy ){
    set_baudrate(baudrate);

    mosi_gpio.outpp();
    sclk_gpio.outpp(HIGH);

    for(auto & cs_gpio : cs_port){
        if(cs_gpio.valid()){
            cs_gpio.outpp(HIGH);
        }
    }

    miso_gpio.inpd();
}


BusError SpiSw::transfer(uint32_t & data_rx, const uint32_t data_tx){
    uint32_t ret = 0;

    sclk_gpio.set();

    for(uint8_t i = 0; i < data_bits; i++){
        sclk_gpio.set();
        delayDur();
        mosi_gpio = bool(data_tx & (1 << (i)));
        delayDur();
        sclk_gpio.clr();
        delayDur();

        if(m_msb){
            mosi_gpio = bool(data_tx & (1 << (data_bits - 2 - i)));
            ret <<= 1; ret |= miso_gpio.read();
            delayDur();
        }else{
            mosi_gpio = bool(data_tx & (1 << (i)));
            ret >>= 1; ret |= (miso_gpio.read() << (data_bits - 1)) ;
            delayDur();
        }
    }

    sclk_gpio.set();

    data_rx = ret;
    return BusError::OK;
}