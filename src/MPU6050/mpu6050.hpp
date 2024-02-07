#ifndef __MPU6050_HPP__

#define __MPU6050_HPP__

#include "../bus/bus_inc.h"

class MPU6050{
protected:
    I2cDrv & bus_drv;

    struct vec3{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct{
        vec3 accel;
        int16_t temperature;
        vec3 gyro;
    };

    enum class RegAddress:uint8_t{
        Accel_x = 0x3b,
        Accel_y = 0x3d,
        Accel_z = 0x3f,
        Temperature = 0x41,
        Gyro_x = 0x43,
        Gyro_y = 0x45,
        Gyro_z = 0x47,
    };

    void writeReg(const uint8_t & reg, const uint8_t data){
        bus_drv.write({reg, data});
    }

    void writeReg(const RegAddress & reg, const uint8_t data){
        bus_drv.write({(uint8_t)reg, data});
    }

    void requestData(const RegAddress & reg, uint8_t * data, const size_t len){
        bus_drv.readPool((uint8_t)reg, data, 2, len);
    }

public:
    MPU6050(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    void init();
    void flush();
    void getAccel();
    // void getGyro();
    void getTemperature();
};


#endif