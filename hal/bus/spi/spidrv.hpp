#pragma once

#include "spi.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>


class SpiDrv:public BusDrv<Spi>{
public:
    SpiDrv(Spi & _bus, const uint8_t _index, const uint32_t _wait_time = 320):BusDrv<Spi>(_bus, _index,_wait_time){;}

    bool m_msb = false;
    uint32_t m_baudrate = 1000000;

    void msb(const bool en){m_msb = en;}
    void baud(const uint32_t baud){m_baudrate = baud;}
};
