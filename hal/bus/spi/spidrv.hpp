#pragma once

#include "spi.hpp"
#include "hal/bus/busdrv.hpp"

#include <type_traits>
#include <concepts>
#include <initializer_list>


namespace ymd::hal{

class SpiDrv:public NonProtocolBusDrv<Spi>{
protected:
    Endian endian_ = LSB;  
    uint32_t baudrate_ = 1000000;
public:
    SpiDrv(hal::Spi & bus, const uint8_t index):NonProtocolBusDrv<Spi>(bus, index){;}

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