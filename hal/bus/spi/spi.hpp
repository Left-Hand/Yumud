#pragma once

#include <optional>
#include "core/utils/Option.hpp"

#include "hal/gpio/vport.hpp"
#include "hal/bus/bus_base.hpp"
#include "hal/bus/bus_enums.hpp"
#include "hal/hal_result.hpp"

namespace ymd::hal{


class SpiSlaveRank{
public:
    explicit constexpr SpiSlaveRank(const uint8_t rank):
        rank_(rank){}

    uint8_t count() const {return rank_;}
    uint16_t as_unique_id() const {return static_cast<uint16_t>(rank_);}

private:
    uint8_t rank_;
};

enum class SpiMode:uint8_t{
    _0,
    _1,
    _2,
    _3
};

enum class SpiClockPolarity:uint8_t{
    IdleLow = 0,
    IdleHigh = 1
};

enum class SpiClockPhase:uint8_t{
    CaptureOnFirst = 0,
    CaptureOnSecond = 1
};

class Spi{
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

    bool is_occupied(){return owner_.is_borrowed();}

    [[nodiscard]] virtual hal::HalResult read(uint32_t & data) = 0;
    [[nodiscard]] virtual hal::HalResult write(const uint32_t data) = 0;
    [[nodiscard]] virtual hal::HalResult transceive(uint32_t & data_rx, const uint32_t data_tx) = 0;

    [[nodiscard]] virtual hal::HalResult set_data_width(const uint8_t bits) = 0;
    [[nodiscard]] virtual hal::HalResult set_baudrate(const uint32_t baud) = 0;
    [[nodiscard]] virtual hal::HalResult set_bitorder(const Endian endian) = 0;


    struct Config{
        uint32_t baudrate;
        SpiMode mode = SpiMode::_3;
        CommStrategy tx_strategy = CommStrategy::Blocking;
        CommStrategy rx_strategy = CommStrategy::Blocking;
    };

    virtual void init(const Config & cfg) = 0;


    [[nodiscard]]
    Option<SpiSlaveRank> allocate_cs_gpio(Some<hal::GpioIntf *> io);

protected:
    VGpioPort <SPI_MAX_PINS> cs_port_ = VGpioPort<SPI_MAX_PINS>();
    CommStrategy tx_strategy_ = CommStrategy::Nil;
    CommStrategy rx_strategy_ = CommStrategy::Nil;
    PeripheralOwnershipTracker owner_ = {};
    Option<Nth> last_nth_ = None;

    [[nodiscard]] __fast_inline hal::HalResult lead(const SpiSlaveRank rank){
        const auto nth = Nth(rank.count());
        if(not cs_port_.is_nth_valid(nth))
            return hal::HalResult::NoSelecter;
        cs_port_[nth].clr();
        last_nth_ = Some(nth);
        return hal::HalResult::Ok();
    }

    __fast_inline void trail(){
        const auto nth = last_nth_.unwrap();
        cs_port_[nth].set();
        last_nth_ = None;
    }

    void bind_cs_gpio(
        Some<hal::GpioIntf *> gpio, 
        const Nth nth
    ){
        gpio.deref().outpp(HIGH);
        cs_port_.bind_pin(gpio.deref(), nth);
    }


};

}