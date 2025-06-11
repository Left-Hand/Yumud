#pragma once

#include "hal/gpio/vport.hpp"
#include "hal/bus/bus_base.hpp"

#include <optional>

namespace ymd::hal{


class SpiSlaveIndex{
    public:
        explicit constexpr SpiSlaveIndex(const uint16_t spi_idx):
            spi_idx_(spi_idx){}
    
        uint8_t as_u8() const {return spi_idx_;}

        LockRequest to_req() const {
            return LockRequest(spi_idx_, 0);
        }
    private:
        uint8_t spi_idx_;
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
        if(not cs_port_.is_index_valid(index))
            return hal::HalResult::NoSelecter;
        cs_port_[index].clr();
        last_index = index;
        return hal::HalResult::Ok();
    }

    __fast_inline void trail(){
        cs_port_[last_index.value()].set();
        last_index.reset();
    }

    void bind_cs_pin(hal::GpioIntf & gpio, const uint8_t index){
        gpio.outpp(HIGH);
        cs_port_.bind_pin(gpio, index);
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

    virtual void init(
        const uint32_t baudrate, 
        const CommStrategy tx_strategy = CommStrategy::Blocking, 
        const CommStrategy rx_strategy = CommStrategy::Blocking) = 0;


    [[nodiscard]]
    std::optional<SpiSlaveIndex> attach_next_cs(hal::GpioIntf & io);
};

}