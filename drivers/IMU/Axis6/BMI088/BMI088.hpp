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


    struct R8_AccConf:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        const uint8_t __resv__:3;
        uint8_t afs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    }DEF_R8(acc_conf_reg)

    struct R8_AccErr:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        uint8_t fatal_err:1;
        const uint8_t __resv__:1;
        uint8_t err_code:3;
        uint8_t xg_st:3;
    }DEF_R8(acc_err_reg)

    struct R8_AccStatus:public Reg8<>{
        scexpr RegAddress address = 0x1c;
        const uint8_t __resv__:7;
        uint8_t drdy_acc:1 = 0;
    }DEF_R8(acc_stat_reg)

    REG8_QUICK_DEF(0x12, AccXReg, acc_x_reg);
    REG8_QUICK_DEF(0x14, AccYReg, acc_y_reg);
    REG8_QUICK_DEF(0x16, AccZReg, acc_z_reg);
    REG8_QUICK_DEF(0x18, SensorTime0, sensor_t0_reg);
    REG8_QUICK_DEF(0x19, SensorTime1, sensor_t1_reg);
    REG8_QUICK_DEF(0x1A, SensorTime2, sensor_t2_reg);

    struct R8_AccIntStatus1:public Reg8<>{
        scexpr RegAddress address = 0x1c;
        const uint8_t __resv__:7 = 0;
        uint8_t acc_drdy:1 = 0;
    }DEF_R8(acc_intstat1_reg)

    REG8_QUICK_DEF(0x22, Temperature, temp_reg);

    struct R8_AccRange:public Reg8<>{
        scexpr RegAddress address = 0x1c;
        uint8_t acc_range:2;
        const uint8_t __resv__:6 = 0;
    }DEF_R8(acc_range_reg)

    struct _R8_IoCtrl:public Reg8<>{
        uint8_t __resv1__:1;
        uint8_t int_lvl:1;
        uint8_t int_od:1;
        uint8_t int_out:1;
        uint8_t int_in:1;
        uint8_t __resv2__:3;
    };

    struct R8_Int1Ctrl:public _R8_IoCtrl{
        scexpr RegAddress address = 0x53;
    }DEF_R8(int1_ctrl_reg)

    struct R8_Int2Ctrl:public _R8_IoCtrl{
        scexpr RegAddress address = 0x54;
    }DEF_R8(int2_ctrl_reg)

    struct R8_Int1MapData:public Reg8<>{
        scexpr RegAddress address = 0x56;
        uint8_t __resv1__:2;
        uint8_t int1_drdy:1;
        uint8_t __resv2__:3;
        uint8_t int2_drdy:1;
        uint8_t __resv3__:1;
    }DEF_R8(int1_map_data_reg)

    REG8_QUICK_DEF(0x6D, AccSelfTest, acc_selftest_reg);
    REG8_QUICK_DEF(0x7c, AccPwrConf, acc_pwrconf_reg);
    REG8_QUICK_DEF(0x7d, AccPwrCtrl, acc_pwrctrl_reg);
    REG8_QUICK_DEF(0x7e, AccSoftReset, acc_softreset_reg);
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
