#pragma once

#include "details/INA228_phy.hpp"

namespace ymd::drivers{

class INA228 final:public INA228_Regs{
public:

    explicit INA228(Some<hal::I2cBase *> i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

private:
    hal::I2cDrv i2c_drv_;
};
}
