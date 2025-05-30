#include "spisw.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiSw::init(const uint32_t baudrate, const CommStrategy tx_strategy , const CommStrategy rx_strategy ){
    set_baudrate(baudrate);

    mosi_gpio.outpp();
    sclk_gpio.outpp(HIGH);

    for(size_t i = 0; i < cs_port_.size(); i++){
        if(cs_port_.is_index_valid(i)){
            cs_port_[i].outpp(HIGH);
        }
    }

    miso_gpio.inpd();
}


hal::HalResult SpiSw::transceive(uint32_t & data_rx, const uint32_t data_tx){
    uint32_t ret = 0;

    sclk_gpio.set();

    for(uint8_t i = 0; i < data_bits; i++){
        sclk_gpio.set();
        delay_dur();
        mosi_gpio = BoolLevel::from(data_tx & (1 << (i)));
        delay_dur();
        sclk_gpio.clr();
        delay_dur();

        if(m_msb){
            mosi_gpio = BoolLevel::from(data_tx & (1 << (data_bits - 2 - i)));
            ret <<= 1; ret |= miso_gpio.read().to_bool();
            delay_dur();
        }else{
            mosi_gpio = BoolLevel::from(data_tx & (1 << i));
            ret >>= 1; ret |= (uint32_t(miso_gpio.read().to_bool()) << (data_bits - 1)) ;
            delay_dur();
        }
    }

    sclk_gpio.set();

    data_rx = ret;
    return hal::HalResult::Ok();
}