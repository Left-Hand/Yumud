#pragma once

#include "drivers/device_defs.h"
#include "hal/timer/timer_oc.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "sys/math/real.hpp"


namespace ymd::drivers{

class AD7607{
protected:
    hal::SpiDrv spi_drv;
    hal::TimerOC & trigger;
    hal::GpioIntf & rst_gpio;
public:
    AD7607(
        hal::SpiDrv && _spi_drv, 
        hal::TimerOC & _trgger, 
        hal::GpioIntf & _rst_gpio = hal::GpioNull
    ):
        spi_drv(std::move(_spi_drv)), 
        trigger(_trgger), 
        rst_gpio(_rst_gpio){;}

    AD7607(
        const hal::SpiDrv & _spi_drv, 
        hal::TimerOC & _trgger, 
        hal::GpioIntf & _rst_gpio = hal::GpioNull
    ):
        spi_drv(_spi_drv), 
        trigger(_trgger), 
        rst_gpio(_rst_gpio){;}

    void reset(void);
    void init(void);
    void start(void);
    void stop(void);
    void BusyIrqCallback(const uint16_t *ad7606Val,uint8_t ad7606Chl);
    real_t conv(uint16_t bin);
};
}