//这个驱动还在推进状态

#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class XL2400{
public:

protected:
    hal::SpiDrv spi_drv;
public:
    XL2400(const hal::SpiDrv & _spi_drv):spi_drv(_spi_drv){;}
    XL2400(hal::SpiDrv && _spi_drv):spi_drv(_spi_drv){;}
};

}