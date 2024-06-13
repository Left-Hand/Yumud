#ifndef __SPI_DRV_HPP__

#define __SPI_DRV_HPP__

#include "spi.hpp"
#include "../busdrv.hpp"
#include <type_traits>
#include <initializer_list>


class SpiDrv:public BusDrv<Spi>{
public:
    SpiDrv(Spi & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):BusDrv<Spi>(_bus, _index,_wait_time){;}


};

#endif