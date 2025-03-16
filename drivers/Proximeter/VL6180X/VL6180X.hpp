#pragma once

#include "drivers/device_defs.h"

namespace ymd{

class VL6180X{
protected:
    hal::I2cDrv i2c_drv_;
    using RegAddress = uint16_t;

    BusError writeReg(const RegAddress command, const uint8_t data);
    BusError readReg(const RegAddress command, uint8_t & data);
public:
    scexpr uint8_t default_i2c_addr = 0;
    VL6180X(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL6180X(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    VL6180X(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_{hal::I2cDrv(i2c, i2c_addr)}{;}
};

}