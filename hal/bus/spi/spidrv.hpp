#pragma once

#include "spi.hpp"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>


namespace ymd::hal{

class SpiSlaveIndex{
public:
    explicit constexpr SpiSlaveIndex(const uint16_t spi_idx):
        spi_idx_(spi_idx){}

    uint8_t as_u8() const {return spi_idx_;}
private:
    uint8_t spi_idx_;
};


class SpiDrv:public NonProtocolBusDrv<Spi>{
protected:
    Endian endian_ = LSB;  
    uint32_t baudrate_ = 1000000;
public:
    SpiDrv(hal::Spi & bus, const SpiSlaveIndex idx):
        NonProtocolBusDrv<Spi>(bus, idx.as_u8()){;}

    template<typename T>
    void force_write(const T data) {
        constexpr size_t size = sizeof(T);
        if (size != 1) this->set_data_width(size * 8);

        if constexpr (size == 1) {
            bus_.write(uint8_t(data));
        } else if constexpr (size == 2) {
            bus_.write(uint16_t(data));
        } else {
            bus_.write(uint32_t(data));
        }

        if (size != 1) this->set_data_width(8);
    }
    
    void set_endian(const Endian _endian){endian_ = _endian;}
    void set_baudrate(const uint32_t baud){baudrate_ = baud;}
};


template<>
struct driver_of_bus<hal::Spi>{
    using driver_type = hal::SpiDrv;
};
    
}