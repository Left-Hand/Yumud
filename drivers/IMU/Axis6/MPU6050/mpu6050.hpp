#ifndef __MPU6050_HPP__

#define __MPU6050_HPP__

#include "drivers/IMU/IMU.hpp"

class MPU6050:public Axis6{
public:
    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

    // enum class DataRate:uint8_t{

    // }
protected:
    I2cDrv & bus_drv;

    struct vec3i{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct{
        vec3i accel;
        uint16_t temprature;
        vec3i gyro;
    };

    real_t accel_scaler = real_t(9.8 * 4);
    real_t gyro_scaler = real_t(0.001064f * 16384);

    enum class RegAddress:uint8_t{
        AccelX = 0x3b,
        AccelY = 0x3d,
        AccelZ = 0x3f,
        Temp = 0x41,
        GyroX = 0x43,
        GyroY = 0x45,
        GyroZ = 0x47,
    };

    void writeReg(const uint8_t & reg_addr, const uint8_t data){
        bus_drv.writeReg(reg_addr, data);
    }

    void writeReg(const RegAddress & reg, const uint8_t data){
        bus_drv.writeReg((uint8_t)reg, data);
    }

    void requestData(const RegAddress & reg_addr, uint8_t * datas, const size_t len){
        bus_drv.readPool((uint8_t)reg_addr, datas, 2, len);
    }

public:
    MPU6050(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    void init();
    void flush();
    void getAccel(real_t & x, real_t & y, real_t & z) override;
    void getGyro(real_t & x, real_t & y, real_t & z) override;
    void getTemperature(real_t & temp);
};


#endif