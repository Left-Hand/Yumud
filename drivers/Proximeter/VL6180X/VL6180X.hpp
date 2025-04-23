#pragma once

#include "core/io/regs.hpp"


#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd{

class VL6180X{
protected:
    hal::I2cDrv i2c_drv_;
    using RegAddress = uint16_t;

    BusError write_reg(const RegAddress command, const uint8_t data);
    BusError read_reg(const RegAddress command, uint8_t & data);
public:
    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0);
    VL6180X(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    VL6180X(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    VL6180X(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_{hal::I2cDrv(i2c, addr)}{;}
};

}