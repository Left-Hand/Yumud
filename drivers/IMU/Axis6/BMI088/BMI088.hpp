#pragma once


#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"


namespace ymd::drivers{

class BMI088:public Axis6{
public:
    scexpr uint8_t default_i2c_addr = 0xd0;


protected:
    std::optional<I2cDrv> i2c_drv_;
    std::optional<SpiDrv> spi_drv_;

    using RegAddress = uint8_t;

    struct AccChipIdReg:public Reg8{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t default_value = 0x1E;
        uint8_t chip_id;
    };

    struct AccErrReg:public Reg8{
        scexpr RegAddress address = 0x02;
        uint8_t fatal_err:1;
        uint8_t :1;
        uint8_t err_code:3;
        uint8_t:3;
    };

    struct AccStatusReg:public Reg8{
        scexpr RegAddress address = 0x03;
        uint8_t :7;
        uint8_t drdy:1;
    };



    void writeReg(const RegAddress reg, const uint8_t data);

    void writeReg(const RegAddress reg, uint8_t & data);

    void requestData(const RegAddress reg_addr, int16_t * datas, const size_t len);
public:
    BMI088(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    BMI088(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    BMI088(I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(I2cDrv{i2c, default_i2c_addr}){;}

    BMI088(const SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    BMI088(SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    BMI088(Spi & spi, const uint8_t index):spi_drv_(SpiDrv{spi, index}){;}


    void init();
    void update();
    std::tuple<real_t, real_t, real_t> getAcc() override;
    std::tuple<real_t, real_t, real_t> getGyr() override;
    real_t getTemperature();
};


};