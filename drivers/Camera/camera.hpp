#ifndef __CAMERA_HPP__

#define __CAMERA_HPP__


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/dvp/dvp.hpp"
#include "hal/gpio/gpio.hpp"

#include "types/vector2/vector2_t.hpp"
#include "types/image/image.hpp"
#include "types/rgb.h"

class SccbDrv:public I2cDrv{
protected:
    using BusDrv<I2c>::index;
    using BusDrv<I2c>::bus;
public:
    SccbDrv(I2c & _bus, const uint8_t & _index, const uint32_t & _wait_time = 320):
        I2cDrv(_bus, _index, _wait_time){};

    void writeSccbReg(const uint8_t reg_address, const uint16_t reg_data){
        if(!bus.begin(index)){
            bus.write(reg_address);
            bus.write((uint8_t)(reg_data >> 8));
            bus.begin(index);
            bus.write(0xF0);
            bus.write((uint8_t)reg_data);
            bus.end();
        }
    }

    void readSccbReg(const uint8_t reg_address, uint16_t & reg_data){
        if(!bus.begin(index)){
            uint32_t data_l, data_h;
            bus.write(reg_address);
            bus.begin(index | 0x01);
            bus.read(data_h, false);

            bus.begin(index);
            bus.write(0xF0);
            bus.begin(index | 0x01);
            bus.read(data_l, false);
            bus.end();

            reg_data = ((uint8_t)data_h << 8) | (uint8_t)data_l;
        }
    }
};

template<typename ColorType>
class CameraWithSccb:public Camera<ColorType>{
protected:
    SccbDrv bus_drv;
    CameraWithSccb(const SccbDrv & _bus_drv, const Vector2i & size):ImageBasics<ColorType>(size), Camera<ColorType>(size), bus_drv(_bus_drv){;}

    void writeReg(const uint8_t addr, const uint16_t data){
        bus_drv.writeSccbReg(addr, data);
    }

    void readReg(const uint8_t addr, uint16_t & data){
        bus_drv.writeSccbReg(addr, data);
    }
};




#endif