#pragma once

#include <optional>

#include "../drivers/device_defs.h"
#include "../drivers/IMU/IMU.hpp"

class BMI160:public Axis6{
public:
    static constexpr uint8_t default_id = 0xd0;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

protected:
    std::optional<I2cDrv> i2c_drv;
    std::optional<SpiDrv> spi_drv;

    struct vec3i{
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct ErrReg{

    };

    static constexpr uint8_t default_chip_id = 0;//TODO
    static constexpr uint8_t default_i2c_addr = 0x68;

    struct{
        uint8_t chip_id_reg;
        uint8_t __resv1__;

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

    void writeReg(const uint8_t reg_addr, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg(reg_addr, data);
    }

    void writeReg(const RegAddress reg, const uint8_t data){
        if(i2c_drv) i2c_drv->writeReg((uint8_t)reg, data);
    }

    void requestData(const RegAddress reg_addr, int16_t * datas, const size_t len){
        if(i2c_drv) i2c_drv->readPool((uint8_t)reg_addr, datas, len);
    }

public:
    void init();
    void update();
    BMI160(I2cDrv & _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2cDrv && _bus_drv):i2c_drv(_bus_drv){;}
    BMI160(I2c & bus):i2c_drv(I2cDrv(bus, default_id)){;}

    std::tuple<real_t, real_t, real_t> getAccel() override;
    std::tuple<real_t, real_t, real_t> getGyro() override;
};
