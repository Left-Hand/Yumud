//这个驱动还在推进状态

#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

class XL2400{
public:
    explicit XL2400(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit XL2400(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
private:
    hal::SpiDrv spi_drv_;
};

}