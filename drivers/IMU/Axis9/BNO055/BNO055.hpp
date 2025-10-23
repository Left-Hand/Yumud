#pragma once

#include "bno055_prelude.hpp"

namespace ymd::drivers{


class BNO055:
    public AccelerometerIntf,
    public GyroscopeIntf,
    public MagnetometerIntf,
    public BNO055_Prelude{
    

private:
    BNO055_Regs regs_ = {};
};
}