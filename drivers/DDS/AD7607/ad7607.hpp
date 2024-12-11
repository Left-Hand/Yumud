#pragma once

#include "../drivers/device_defs.h"
#include "../hal/timer/timer_oc.hpp"
#include "../hal/timer/instance/timer_hw.hpp"
#include "sys/math/real.hpp"


namespace ymd::drivers{

class AD7607{
protected:
    SpiDrv spi_drv;
    TimerOC & trigger;
    Gpio & rst_gpio;
public:
    AD7607(SpiDrv && _spi_drv, TimerOC & _trgger, Gpio & _rst_gpio = GpioNull):spi_drv(_spi_drv), trigger(_trgger), rst_gpio(_rst_gpio){;}

    void reset(void);
    void init(void);
    void start(void);
    void stop(void);
    void BusyIrqCallback(uint16_t *ad7606Val,uint8_t ad7606Chl);
    real_t conv(uint16_t bin);
};
}