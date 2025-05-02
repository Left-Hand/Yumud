#pragma once

#include "details/INA228_phy.hpp"

namespace ymd::drivers{

class INA228 final:public INA228_Regs{
public:

    INA228(hal::I2c & i2c, hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}

private:
    hal::I2cDrv phy_;
};
}
