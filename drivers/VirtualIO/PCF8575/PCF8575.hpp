#pragma once
#include "drivers/device_defs.h"

namespace yumud::drivers{
class PCF8575{
protected:
    I2cDrv i2c_drv_;

    uint8_t data;
public:
    scexpr uint8_t default_i2c_addr = 0b01000000;

    PCF8575(I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCF8575(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    PCF8575(I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(I2cDrv{i2c, default_i2c_addr}){;}
    // void write(const uint8_t _data){
    //     data = _data;
    //     i2c_drv_.write(data);
    // }

    // uint8_t read(){
    //     uint16_t ret = 0;
    //     i2c_drv_.readMulti();
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