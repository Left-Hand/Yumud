#pragma once


#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/BoschIMU.hpp"

namespace ymd::drivers{

class BMI088:public Axis6, public BoschSensor{
public:
    scexpr uint8_t default_i2c_addr = 0xd0;


protected:

    using RegAddress = uint8_t;

    struct AccChipIdReg:public Reg8<>{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t default_value = 0x1E;
        uint8_t chip_id;
    };

    struct AccErrReg:public Reg8<>{
        scexpr RegAddress address = 0x02;
        uint8_t fatal_err:1;
        uint8_t :1;
        uint8_t err_code:3;
        uint8_t:3;
    };

    struct AccStatusReg:public Reg8<>{
        scexpr RegAddress address = 0x03;
        uint8_t :7;
        uint8_t drdy:1;
    };

public:
    BMI088(const hal::I2cDrv & i2c_drv):BoschSensor(i2c_drv){;}
    BMI088(hal::I2cDrv && i2c_drv):BoschSensor(std::move(i2c_drv)){;}
    BMI088(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):BoschSensor(hal::I2cDrv{i2c, default_i2c_addr}){;}

    BMI088(const hal::SpiDrv & spi_drv):BoschSensor(spi_drv){;}
    BMI088(hal::SpiDrv && spi_drv):BoschSensor(std::move(spi_drv)){;}
    BMI088(hal::Spi & spi, const uint8_t index):BoschSensor(hal::SpiDrv{spi, index}){;}


    void init();
    void update();
    Option<Vector3R> getAcc();
    Option<Vector3R> getGyr();
    real_t getTemperature();
};


};