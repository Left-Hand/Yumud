#ifndef __SGM58031_HPP__

#define __SGM58031_HPP__

#include "../drivers/device_defs.h"
#include "real.hpp"

#define AD9854_DEBUG

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#define AD9854_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define AD9854_DEBUG(...)
#endif

class AD9854{
public:
    struct Ports{
        GpioConcept & IO_RESET;
        GpioConcept & MRESET;
        GpioConcept & UD_CLK;
        GpioConcept & F_B_H;
        GpioConcept & OSK;
    };

protected:
    SpiDrv & bus_drv;
    Ports ports;

public:
    AD9854(SpiDrv & _bus_drv, const Ports & _ports):bus_drv(_bus_drv), ports(_ports){;}
    
    void Init(void);
    void SendOneByte(uint8_t data);
    void SendData(uint8_t _register, uint8_t* data, uint8_t ByteNum);
    void SetFreq(real_t fre);
};

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#endif

#endif