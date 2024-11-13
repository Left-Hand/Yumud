#pragma once

#include "spi.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>


namespace yumud{

class SpiDrv:public NonProtocolBusDrv<Spi>{
protected:
    Endian m_endian = LSB;  
    uint32_t m_baudrate = 1000000;
public:
    SpiDrv(Spi & _bus, const uint8_t _index):NonProtocolBusDrv<Spi>(_bus, _index){;}

    template<typename T>
    void forceWrite(const T data) {
        constexpr size_t size = sizeof(T);
        if (size != 1) this->setDataBits(size * 8);

        if constexpr (size == 1) {
            bus.write((uint8_t)data);
        } else if constexpr (size == 2) {
            bus.write(uint16_t(data));
        } else {
            bus.write(uint32_t(data));
        }

        if (size != 1) this->setDataBits(8);
    }
    
    void setEndian(const Endian _endian){m_endian = _endian;}
    void setBaud(const uint32_t baud){m_baudrate = baud;}
};


namespace internal{
template <>
struct DrvOfBus<Spi>{
    using DrvType = SpiDrv;
};

}
};