#pragma once

#include "aw32001_prelude.hpp"

namespace ymd::drivers{

struct AW32001 final:public AW32001_Prelude{ 
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x34)
private:
    hal::I2cDrv i2c_drv_;
};
}