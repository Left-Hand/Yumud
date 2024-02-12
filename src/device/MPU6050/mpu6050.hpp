#ifndef __MPU6050_HPP__

#define __MPU6050_HPP__

#include "device_defs.h"
#include "types/real.hpp"

class MPU6050{
protected:
    I2cDrv & bus_drv;

    struct AccelXReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct AccelYReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct AccelZReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct TempReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct GyroXReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct GyroYReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct GyroZReg:public Reg16{
        REG16_BEGIN
        REG16_END
    };

    struct{
        AccelXReg accelXReg;
        AccelYReg accelYReg;
        AccelZReg accelZReg;
        TempReg tempReg;
        GyroXReg gyroXReg;
        GyroYReg gyroYReg;
        GyroZReg gyroZReg;
    };

    real_t accel_scaler;
    real_t gyro_scaler;
    enum class RegAddress:uint8_t{
        AccelX = 0x3b,
        AccelY = 0x3d,
        AccelZ = 0x3f,
        Temp = 0x41,
        GyroX = 0x43,
        GyroY = 0x45,
        GyroZ = 0x47,
    };

    int16_t Reg16ToI16(const Reg16 & reg){
        return *(int16_t *)&(reg);
    }

    void writeReg(const uint8_t & reg_addr, const uint8_t data){
        bus_drv.writeReg(reg_addr, data);
    }

    void writeReg(const RegAddress & reg, const uint8_t data){
        bus_drv.writeReg((uint8_t)reg, data);
    }

    void requestData(const RegAddress & reg_addr, Reg16 & reg, const size_t len){
        bus_drv.readPool((uint8_t)reg_addr, (uint8_t *)&reg, 2, len);
    }

public:
    MPU6050(I2cDrv & _bus_drv):bus_drv(_bus_drv){;}
    void init();
    void flush();
    void getAccel(real_t & x, real_t & y, real_t & z);
    void getGyro(real_t & x, real_t & y, real_t & z);
    void getTemperature(real_t & temp);
};


#endif