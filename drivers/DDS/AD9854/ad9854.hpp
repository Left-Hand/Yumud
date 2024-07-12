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

protected:
    SpiDrv & bus_drv;
    struct{
        Gpio & IO_RESET = portD[2];
        Gpio & MRESET = portD[6];
        Gpio & UD_CLK = portD[7];
        Gpio & F_B_H = portE[5];
        Gpio & OSK = portE[6];
    };

public:
    AD9854(SpiDrv & _bus_drv):bus_drv(_bus_drv){;}
    
    void Init(void);
    void SendOneByte(uint8_t data);
    void SendData(uint8_t _register, uint8_t* data, uint8_t ByteNum);
    void SetFreq(real_t fre);
};

#ifdef AD9854_DEBUG
#undef AD9854_DEBUG
#endif

#endif