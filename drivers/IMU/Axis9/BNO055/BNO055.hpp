#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct BNO055_Collections{

};

struct BNO055_Regs:public BNO055_Collections{
};

class BNO055:
    public AccelerometerIntf,
    public GyroscopeIntf,
    public MagnetometerIntf,
    public BNO055_Regs{
    
};


}