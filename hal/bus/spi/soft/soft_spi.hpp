#pragma once

#include "../spi_base.hpp"
#include "core/clock/clock.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{

class SoftSpi final: public SpiBase{
protected:
    volatile int8_t occupied = -1;
    hal::Gpio & sclk_pin_;
    hal::Gpio & mosi_pin_;
    hal::Gpio & miso_pin_;

    uint16_t delays = 100;
    SpiWordSize wordsize_ = SpiWordSize::OneByte;
    bool is_msb_ = true;

    __no_inline void delay_dur(){
        clock::delay(Microseconds(delays));
    }
    hal::HalResult lead(const SpiSlaveRank rank) {
        auto ret = SpiBase::lead(rank);
        delay_dur();
        return ret;
    }

    void trail() {
        sclk_pin_.set_high();
        delay_dur();
        SpiBase::trail();
    }
protected :
public:

    SoftSpi(
        Some<hal::Gpio *> sclk_gpio,
        Some<hal::Gpio *> mosi_gpio,
        Some<hal::Gpio *> miso_gpio
    ):
        sclk_pin_(sclk_gpio.deref()),
        mosi_pin_(mosi_gpio.deref()), 
        miso_pin_(miso_gpio.deref()){;}

    SoftSpi(
        Some<hal::Gpio *> sclk_gpio,
        Some<hal::Gpio *> mosi_gpio,
        Some<hal::Gpio *> miso_gpio,
        Some<hal::Gpio *> cs_gpio
    ):
        SoftSpi(sclk_gpio, mosi_gpio, miso_gpio)
    {
        bind_cs_pin(cs_gpio.get(), 0_nth);
    }

    SoftSpi(const SoftSpi &) = delete;
    SoftSpi(SoftSpi &&) = delete;

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
    hal::HalResult set_wordsize(const SpiWordSize wordsize)  {
        wordsize_ = wordsize;
        return HalResult::Ok();
    }

    hal::HalResult set_bitorder(const BitOrder bit_order)  {
        is_msb_ = (bit_order == MSB);
        return HalResult::Ok();
    }
};



}