#pragma once

#include "spi.hpp"
#include "core/clock/clock.hpp"

namespace ymd::hal{

class SpiSw final: public Spi{
protected:
    volatile int8_t occupied = -1;
    hal::GpioIntf & sclk_pin_;
    hal::GpioIntf & mosi_pin_;
    hal::GpioIntf & miso_pin_;

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
        sclk_pin_.set_high();
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
        sclk_pin_(sclk_gpio.deref()),
        mosi_pin_(mosi_gpio.deref()), 
        miso_pin_(miso_gpio.deref()){;}

    SpiSw(
        Some<hal::GpioIntf *> sclk_gpio,
        Some<hal::GpioIntf *> mosi_gpio,
        Some<hal::GpioIntf *> miso_gpio,
        Some<hal::GpioIntf *> cs_gpio
    ):
        SpiSw(sclk_gpio, mosi_gpio, miso_gpio)
    {
        bind_cs_pin(cs_gpio, 0_nth);
    }

    SpiSw(const SpiSw &) = delete;
    SpiSw(SpiSw &&) = delete;

    void init(const SpiConfig & cfg) ;

    hal::HalResult blocking_write(const uint32_t data) {
        uint32_t dummy;
        blocking_transceive(dummy, data);
        return hal::HalResult::Ok();
    }

    hal::HalResult blocking_read(uint32_t & data) {
        uint32_t ret;
        static constexpr uint32_t dummy = 0;
        blocking_transceive(ret, dummy); 
        return hal::HalResult::Ok();
    }

    hal::HalResult blocking_transceive(uint32_t & data_rx, const uint32_t data_tx)  ;

    hal::HalResult set_baudrate(const SpiBaudrate baud);
    hal::HalResult set_word_width(const uint8_t bits)  {
        width_ = bits;
        return HalResult::Ok();
    }

    hal::HalResult set_bitorder(const BitOrder bit_order)  {
        is_msb_ = (bit_order == MSB);
        return HalResult::Ok();
    }
};



}