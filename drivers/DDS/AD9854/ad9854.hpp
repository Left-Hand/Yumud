#pragma once

#include "drivers/device_defs.h"
#include "core/math/real.hpp"

#define AD9854_DEBUG

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#define AD9854_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define AD9854_DEBUG(...)
#endif


namespace ymd::drivers{

class AD9854{
public:
    struct Ports{
        hal::GpioIntf & IO_RESET;
        hal::GpioIntf & MRESET;
        hal::GpioIntf & UD_CLK;
        hal::GpioIntf & F_B_H;
        hal::GpioIntf & OSK;
    };

protected:
    hal::SpiDrv spi_drv_;
    Ports ports;

public:
    AD9854(const hal::SpiDrv & spi_drv, const Ports & _ports):spi_drv_(spi_drv), ports(_ports){;}
    AD9854(hal::SpiDrv && spi_drv, const Ports & _ports):spi_drv_(spi_drv), ports(_ports){;}
    
    void Init(void);
    void SendOneByte(uint8_t data);
    void SendData(uint8_t _register, uint8_t* data, uint8_t ByteNum);
    void SetFreq(real_t fre);
};
}

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#endif
