//这个驱动还未完成

#pragma once

#include "mpr121_prelude.hpp"
namespace ymd::drivers{

class MPR121:public MPR121_Prelude{

public: 
    explicit MPR121(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    explicit MPR121(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    explicit MPR121(Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_{hal::I2cDrv{i2c, addr}}{;}

private:
    hal::I2cDrv i2c_drv_;



};

}