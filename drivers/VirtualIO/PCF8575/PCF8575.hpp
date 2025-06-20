//这个驱动还未完成

#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{
class PCF8575{
protected:
    hal::I2cDrv i2c_drv_;

    uint8_t data;
public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b01000000);

    PCF8575(hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    PCF8575(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    PCF8575(
        hal::I2c & i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

};
}