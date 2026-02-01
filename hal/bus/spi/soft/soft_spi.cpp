#include "soft_spi.hpp"

using namespace ymd;
using namespace ymd::hal;

void SoftSpi::init(const SpiConfig & cfg){
    set_baudrate(cfg.baudrate);

    mosi_pin_.outpp();
    sclk_pin_.outpp(HIGH);

    for(size_t i = 0; i < cs_port_.size(); i++){
        const auto nth = Nth(i);
        if(cs_port_.is_nth_valid(nth)){
            cs_port_[nth].outpp(HIGH);
        }
    }

    miso_pin_.inpd();
}


hal::HalResult SoftSpi::blocking_transceive(uint32_t & data_rx, const uint32_t data_tx){
    uint32_t ret = 0;

    sclk_pin_.set_high();

    const auto num_bits = [&] -> size_t{
        switch(wordsize_){
            case SpiWordSize::OneByte: return 8;
            case SpiWordSize::TwoBytes: return 16;
        }
        __builtin_trap();
    }();

    for(uint8_t i = 0; i < num_bits; i++){
        sclk_pin_.set_high();
        delay_dur();
        mosi_pin_.write(BoolLevel::from(data_tx & (1 << (i))));
        delay_dur();
        sclk_pin_.set_low();
        delay_dur();

        if(is_msb_){
            mosi_pin_.write(BoolLevel::from(data_tx & (1 << (num_bits - 2 - i))));
            ret <<= 1; ret |= miso_pin_.read().to_bool();
            delay_dur();
        }else{
            mosi_pin_.write(BoolLevel::from(data_tx & (1 << i)));
            ret >>= 1; ret |= (uint32_t(miso_pin_.read().to_bool()) << (num_bits - 1)) ;
            delay_dur();
        }
    }

    sclk_pin_.set_high();

    data_rx = ret;
    return hal::HalResult::Ok();
}

hal::HalResult SoftSpi::set_baudrate(const SpiBaudrate baud) {
    const auto baud_freq = [&]{
        if(not baud.is<LeastFreq>()) 
            __builtin_trap();
        return baud.unwrap_as<LeastFreq>().count;
    }();
    if(baud_freq == 0){
        delays = 0;
    }else{
        uint32_t b = baud_freq / 1000;
        delays = 200 / b;
    }
    return HalResult::Ok();
}
