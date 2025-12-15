#pragma once

#include <optional>
#include "core/utils/Option.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/bus_base.hpp"
#include "hal/bus/bus_enums.hpp"
#include "primitive/hal_result.hpp"
#include "spi_primitive.hpp"

namespace ymd::hal{


class [[nodiscard]] Spi{
public:

    #ifndef SPI_MAX_PINS
    static constexpr size_t SPI_MAX_PINS = 4;
    #endif

public:
    Spi(){;}
    Spi(const hal::Spi &) = delete;
    Spi(hal::Spi &&) = delete;


    HalResult borrow(const SpiSlaveRank rank){
        if(false == owner_.is_borrowed()){
            owner_.borrow(rank);
            return lead(rank);
        }else if(owner_.is_borrowed_by(rank)){
            owner_.borrow(rank);
            return lead(rank);
        }else{
            return hal::HalResult::OccuipedByOther;
        }
    }

    void lend(){
        this->trail();
        owner_.lend();
    }

    [[nodiscard]] bool is_occupied(){return owner_.is_borrowed();}

    [[nodiscard]] virtual hal::HalResult blocking_read(uint32_t & data) = 0;
    [[nodiscard]] virtual hal::HalResult blocking_write(const uint32_t data) = 0;
    [[nodiscard]] virtual hal::HalResult blocking_transceive(uint32_t & data_rx, const uint32_t data_tx) = 0;

    [[nodiscard]] virtual hal::HalResult set_word_width(const uint8_t bits) = 0;
    [[nodiscard]] virtual hal::HalResult set_baudrate(const SpiBaudrate baud) = 0;
    [[nodiscard]] virtual hal::HalResult set_bitorder(const BitOrder bitorder) = 0;


    [[nodiscard]]
    Option<SpiSlaveRank> allocate_cs_pin(Some<hal::GpioIntf *> io);

protected:
    VGpioPort <SPI_MAX_PINS> cs_port_ = VGpioPort<SPI_MAX_PINS>();
    PeripheralOwnershipTracker owner_ = {};
    Option<Nth> last_nth_ = None;

    [[nodiscard]] __fast_inline hal::HalResult lead(const SpiSlaveRank rank){
        const auto nth = Nth(rank.count());
        if(not cs_port_.is_nth_valid(nth))
            return hal::HalResult::NoSelecter;
        cs_port_[nth].set_low();
        last_nth_ = Some(nth);
        return hal::HalResult::Ok();
    }

    __fast_inline void trail(){
        const auto nth = last_nth_.unwrap();
        cs_port_[nth].set_high();
        last_nth_ = None;
    }

    void bind_cs_pin(
        Some<hal::GpioIntf *> gpio, 
        const Nth nth
    ){
        gpio.deref().outpp(HIGH);
        cs_port_.bind_pin(gpio.deref(), nth);
    }


};

}