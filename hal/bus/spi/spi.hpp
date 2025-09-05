#pragma once

#include "hal/gpio/vport.hpp"
#include "hal/bus/bus_base.hpp"

#include <optional>
#include "core/utils/Option.hpp"

namespace ymd::hal{


class SpiSlaveIndex{
public:
    explicit constexpr SpiSlaveIndex(const uint8_t spi_idx):
        spi_idx_(spi_idx){}

    uint8_t as_u8() const {return spi_idx_;}

    LockRequest to_req() const {
        return LockRequest(spi_idx_, 0);
    }
private:
    uint8_t spi_idx_;
};

enum class SpiMode:uint8_t{
    _0,
    _1,
    _2,
    _3
};

class Spi:public BusBase{
public:

    #ifndef SPI_MAX_PINS
    static constexpr size_t SPI_MAX_PINS = 4;
    #endif

protected:
    VGpioPort <SPI_MAX_PINS> cs_port_ = VGpioPort<SPI_MAX_PINS>();
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;
    std::optional<uint8_t> last_index;

    [[nodiscard]] __fast_inline hal::HalResult lead(const LockRequest req){
        const auto index = req.id();
        const auto nth = Nth(index);
        if(not cs_port_.is_nth_valid(nth))
            return hal::HalResult::NoSelecter;
        cs_port_[nth].clr();
        last_index = index;
        return hal::HalResult::Ok();
    }

    __fast_inline void trail(){
        const auto nth = Nth(last_index.value());
        cs_port_[nth].set();
        last_index.reset();
    }

    void bind_cs_gpio(
        Some<hal::GpioIntf *> gpio, 
        const uint8_t index
    ){
        gpio.deref().outpp(HIGH);
        cs_port_.bind_pin(gpio.deref(), Nth(index));
    }
public:
    Spi(){;}
    Spi(const hal::Spi &) = delete;
    Spi(hal::Spi &&) = delete;

    
    [[nodiscard]] virtual hal::HalResult read(uint32_t & data) = 0;
    [[nodiscard]] virtual hal::HalResult write(const uint32_t data) = 0;
    [[nodiscard]] virtual hal::HalResult transceive(uint32_t & data_rx, const uint32_t data_tx) = 0;

    [[nodiscard]] virtual hal::HalResult set_data_width(const uint8_t len) = 0;
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
    Option<SpiSlaveIndex> allocate_cs_gpio(Some<hal::GpioIntf *> io);
};

}