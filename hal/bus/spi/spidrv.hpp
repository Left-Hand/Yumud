#pragma once

#include "spi.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>


class SpiDrv:public BusDrv<Spi>{
protected:
    Endian m_endian = LSB;  
    uint32_t m_baudrate = 1000000;
public:
    SpiDrv(Spi & _bus, const uint8_t _index, const uint32_t _wait_time = 320):BusDrv<Spi>(_bus, _index,_wait_time){;}

    void setEndian(const Endian _endian){m_endian = _endian;}
    void setBaud(const uint32_t baud){m_baudrate = baud;}
};
