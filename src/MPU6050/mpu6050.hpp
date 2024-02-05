#ifndef __MPU6050_HPP__

#define __MPU6050_HPP__

#include "../bus/busdrv.hpp"
#include "../bus/uart/uart1.hpp"

class MPU6050{
protected:
    BusDrv & busdrv;

    struct vec3{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct __attribute__((packed)){
        vec3 accel;
        int16_t temprature;
        vec3 gyro;
    };

    enum class Reg:uint8_t{
        Accel_x = 0x3b,
        Accel_y = 0x3d,
        Accel_z = 0x3f,
        Temprature = 0x41,
        Gyro_x = 0x43,
        Gyro_y = 0x45,
        Gyro_z = 0x47,
    };

    void writeReg(const uint8_t & reg, const uint8_t data){
        busdrv.write({reg, data});
    }

    void writeReg(const Reg & reg, const uint8_t data){
        busdrv.write({(uint8_t)reg, data});
    }

    void requestData(const Reg & reg, uint8_t * data, const size_t len){
        busdrv.readPool((uint8_t)reg, data, 2, len);
    }

public:
    MPU6050(BusDrv & _busdrv):busdrv(_busdrv){;}
    void init();
    void reflash();
    void getAccel();
    // void getGyro();
    void getTemprature();
};


#endif