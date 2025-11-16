#pragma once

#include "spi.hpp"
#include "core/clock/clock.hpp"

namespace ymd::hal{

class SpiSw final: public Spi{
protected:
    volatile int8_t occupied = -1;
    hal::GpioIntf & sclk_gpio_;
    hal::GpioIntf & mosi_gpio_;
    hal::GpioIntf & miso_gpio_;

    uint16_t delays = 100;
    uint8_t width_ = 8;
    bool is_msb_ = true;

    __no_inline void delay_dur(){
        clock::delay(Microseconds(delays));
    }
    hal::HalResult lead(const SpiSlaveRank rank) {
        auto ret = Spi::lead(rank);
        delay_dur();
        return ret;
    }

    void trail() {
        sclk_gpio_.set();
        delay_dur();
        Spi::trail();
    }
protected :
public:

    SpiSw(
        Some<hal::GpioIntf *> sclk_gpio,
        Some<hal::GpioIntf *> mosi_gpio,
        Some<hal::GpioIntf *> miso_gpio
    ):
        sclk_gpio_(sclk_gpio.deref()),
        mosi_gpio_(mosi_gpio.deref()), 
        miso_gpio_(miso_gpio.deref()){;}

    SpiSw(
        Some<hal::GpioIntf *> sclk_gpio,
        Some<hal::GpioIntf *> mosi_gpio,
        Some<hal::GpioIntf *> miso_gpio,
        Some<hal::GpioIntf *> cs_gpio
    ):
        SpiSw(sclk_gpio, mosi_gpio, miso_gpio)
    {
        bind_cs_gpio(cs_gpio, 0_nth);
    }

    SpiSw(const SpiSw &) = delete;
    SpiSw(SpiSw &&) = delete;

    void init(const Config & cfg) ;

    hal::HalResult write(const uint32_t data) {
        uint32_t dummy;
        transceive(dummy, data);
        return hal::HalResult::Ok();
    }

    hal::HalResult read(uint32_t & data) {
        uint32_t ret;
        static constexpr uint32_t dummy = 0;
        transceive(ret, dummy); 
        return hal::HalResult::Ok();
    }

    hal::HalResult transceive(uint32_t & data_rx, const uint32_t data_tx)  ;

    hal::HalResult set_baudrate(const uint32_t baudrate) {
        if(baudrate == 0){
            delays = 0;
        }else{
            uint32_t b = baudrate / 1000;
            delays = 200 / b;
        }
        return HalResult::Ok();
    }

    hal::HalResult set_data_width(const uint8_t bits)  {
        width_ = bits;
        return HalResult::Ok();
    }

    hal::HalResult set_bitorder(const BitOrder bit_order)  {
        is_msb_ = (bit_order == MSB);
        return HalResult::Ok();
    }
};



}