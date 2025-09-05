#include "spisw.hpp"

using namespace ymd;
using namespace ymd::hal;

void SpiSw::init(const Config & cfg){
    set_baudrate(cfg.baudrate);

    mosi_gpio_.outpp();
    sclk_gpio_.outpp(HIGH);

    for(size_t i = 0; i < cs_port_.size(); i++){
        const auto nth = Nth(i);
        if(cs_port_.is_nth_valid(nth)){
            cs_port_[nth].outpp(HIGH);
        }
    }

    miso_gpio_.inpd();
}


hal::HalResult SpiSw::transceive(uint32_t & data_rx, const uint32_t data_tx){
    uint32_t ret = 0;

    sclk_gpio_.set();

    for(uint8_t i = 0; i < data_bits; i++){
        sclk_gpio_.set();
        delay_dur();
        mosi_gpio_ = BoolLevel::from(data_tx & (1 << (i)));
        delay_dur();
        sclk_gpio_.clr();
        delay_dur();

        if(m_msb){
            mosi_gpio_ = BoolLevel::from(data_tx & (1 << (data_bits - 2 - i)));
            ret <<= 1; ret |= miso_gpio_.read().to_bool();
            delay_dur();
        }else{
            mosi_gpio_ = BoolLevel::from(data_tx & (1 << i));
            ret >>= 1; ret |= (uint32_t(miso_gpio_.read().to_bool()) << (data_bits - 1)) ;
            delay_dur();
        }
    }

    sclk_gpio_.set();

    data_rx = ret;
    return hal::HalResult::Ok();
}