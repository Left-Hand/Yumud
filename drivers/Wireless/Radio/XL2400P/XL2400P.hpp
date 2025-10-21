//这个驱动还在推进状态

#pragma once

#include "xl2400p_prelude.hpp"

namespace ymd::drivers{

class XL2400P{
public:
    explicit XL2400P(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit XL2400P(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

private:
    hal::SpiDrv spi_drv_;

};

}