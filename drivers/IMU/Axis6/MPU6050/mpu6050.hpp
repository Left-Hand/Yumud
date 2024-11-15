#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers{

class MPU6050:public Axis6{
public:
    scexpr uint8_t default_i2c_addr = 0xd0;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

protected:
    I2cDrv i2c_drv_;

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

    enum RegAddress:uint8_t{
        AccelX = 0x3b,
        AccelY = 0x3d,
        AccelZ = 0x3f,
        Temp = 0x41,
        GyroX = 0x43,
        GyroY = 0x45,
        GyroZ = 0x47,
    };

    void writeReg(const uint8_t addr, const uint8_t data){
        i2c_drv_.writeReg((uint8_t)addr, data, MSB);
    }

    void requestData(const uint8_t reg_addr, int16_t * datas, const size_t len){
        i2c_drv_.readMulti((uint8_t)reg_addr, datas, len, MSB);
    }

public:
    MPU6050(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    MPU6050(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    MPU6050(I2c & bus, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(bus, i2c_addr){;}

    void init();
    void update();
    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
    real_t getTemperature();
};


};