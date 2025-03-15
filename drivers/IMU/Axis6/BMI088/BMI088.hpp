#pragma once


#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/BoschIMU.hpp"

// https://blog.csdn.net/weixin_44080304/article/details/125065724

namespace ymd::drivers{

class BMI088:public Axis6, public BoschSensor{
public:
    scexpr uint8_t default_i2c_addr = 0xd0;

    using Error = BusError;

protected:

    using RegAddress = uint8_t;

    struct R8_AccChipId:public Reg8<>{
        scexpr RegAddress address = 0x00;
        scexpr uint8_t default_value = 0x1E;

        uint8_t chip_id;
    };
    // DEF_R8(R8_AccChipId, acc_chip_id_reg)

    struct R8_AccErr:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t fatal_err:1;
        uint8_t :1;
        uint8_t err_code:3;
        uint8_t:3;
    };
    // DEF_R8(R8_AccErr, acc_err_reg)

    struct R8_AccStatus:public Reg8<>{
        scexpr RegAddress address = 0x03;
        uint8_t :7;
        uint8_t drdy:1;
    };
    // DEF_R8(R8_AccStatus, acc_status_reg)

    // REG16I_QUICK_DEF(0x3B, AccXReg, acc_x_reg);
    // REG16I_QUICK_DEF(0x3D, AccYReg, acc_y_reg);
    // REG16I_QUICK_DEF(0x3F, AccZReg, acc_z_reg);

    // REG16_QUICK_DEF(0x41, TemperatureReg, temperature_reg);

    // REG16I_QUICK_DEF(0x43, GyrXReg, gyr_x_reg);
    // REG16I_QUICK_DEF(0x45, GyrYReg, gyr_y_reg);
    // REG16I_QUICK_DEF(0x47, GyrZReg, gyr_z_reg);
    // struct R8_AccConf2:public Reg8<>{
    //     scexpr RegAddress address = 0x1c;

    //     uint8_t a_dlpf_cfg: 3;
    //     uint8_t accel_fs_hoice_b:1;
    //     const uint8_t __resv__:3 = 0;

    // } acc_conf2_reg = {};
    

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

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::BMI088::Error, BusError> {
        static Result<T, drivers::BMI088::Error> convert(const BusError berr){
            using Error = drivers::BMI088::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return BMI088::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}
