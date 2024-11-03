#pragma once
#include "device_defs.h"

namespace yumud::drivers{
class PCF8574{
protected:
    I2cDrv i2c_drv_;

    uint8_t data;
public:
    PCF8574(I2cDrv & i2c_drv_):i2c_drv_(i2c_drv_){;}
    PCF8574(I2cDrv && i2c_drv_):i2c_drv_(i2c_drv_){;}
    void write(const uint8_t _data){
        data = _data;
        i2c_drv_.write(data);
    }

    uint8_t read(){
        uint8_t ret = 0;
        i2c_drv_.read(ret);
        return ret;
    }

    void writeBit(const uint16_t index, const bool value){
        if(value) data |= (1 << index);
        else data &= (~(1 << index));
        i2c_drv_.write(data);
    }

    bool readBit(const uint16_t index){
        return (read() & (1 << index));
    }
};
}