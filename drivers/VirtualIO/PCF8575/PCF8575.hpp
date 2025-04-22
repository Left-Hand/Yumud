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

    PCF8575(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCF8575(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PCF8575(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):i2c_drv_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}
    // void write(const uint8_t _data){
    //     data = _data;
    //     i2c_drv_.write(data);
    // }

    // uint8_t read(){
    //     uint16_t ret = 0;
    //     i2c_drv_.read_burst();
    //     return ret;
    // }

    // void writeBit(const uint16_t index, const bool value){
    //     if(value) data |= (1 << index);
    //     else data &= (~(1 << index));
    //     i2c_drv_.write(data);
    // }

    // bool readBit(const uint16_t index){
    //     return (read() & (1 << index));
    // }
};
}