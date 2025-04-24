//这个驱动已经完成
//这个驱动还未测试

#pragma once
#include "device_defs.h"

namespace ymd::drivers{
class PCF8574{
protected:
    hal::I2cDrv i2c_drv_;

    uint8_t data;
public:
    PCF8574(hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    PCF8574(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    void write(const uint8_t _data){
        data = _data;
        i2c_drv_.write(data);
    }

    uint8_t read(){
        uint8_t ret = 0;
        i2c_drv_.read(ret);
        return ret;
    }

    void write_bit(const uint16_t index, const bool value){
        if(value) data |= (1 << index);
        else data &= (~(1 << index));
        i2c_drv_.write(data);
    }

    bool read_bit(const uint16_t index){
        return (read() & (1 << index));
    }
};
}