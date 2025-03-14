#pragma once

#include "drivers/device_defs.h"
#include "drivers/IMU/IMU.hpp"

namespace ymd::drivers{

class MPU6050:public Axis6{
public:
    enum class Package:uint8_t{
        MPU6050 = 0x68,
        MPU6500 = 0x70,
        MPU9250 = 0x71
    };

    using Error = BusError;

    scexpr uint8_t default_i2c_addr = 0xd0;

    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class AccRange:uint8_t{
        _2G     =   0,
        _4G     =   1,
        _8G     =   2,
        _16G    =   3
    };

    enum class GyrRange:uint8_t{
        _250deg     =   0,
        _500deg     =   1,
        _1000deg    =   2,
        _2000deg    =   3
    };

protected:
    using I2cDrvProxy = std::optional<hal::I2cDrv>;
    using SpiDrvProxy = std::optional<hal::SpiDrv>;
    I2cDrvProxy p_i2c_drv_ = std::nullopt;
    SpiDrvProxy p_spi_drv_ = std::nullopt;

    using RegAddress = uint8_t;    

    struct GyrConfReg:public Reg8<>{
        scexpr RegAddress address = 0x1b;

        const uint8_t __resv__:3 = 0;
        uint8_t fs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } gyr_conf_reg = {};
    

    struct AccConfReg:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        const uint8_t __resv__:3 = 0;
        uint8_t afs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } acc_conf_reg = {};

    struct AccConf2Reg:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        uint8_t a_dlpf_cfg: 3;
        uint8_t accel_fs_hoice_b:1;
        const uint8_t __resv__:3 = 0;

    } acc_conf2_reg = {};
    
    REG16I_QUICK_DEF(0x3B, AccXReg, acc_x_reg);
    REG16I_QUICK_DEF(0x3D, AccYReg, acc_y_reg);
    REG16I_QUICK_DEF(0x3F, AccZReg, acc_z_reg);

    REG16_QUICK_DEF(0x41, TemperatureReg, temperature_reg);

    REG16I_QUICK_DEF(0x43, GyrXReg, gyr_x_reg);
    REG16I_QUICK_DEF(0x45, GyrYReg, gyr_y_reg);
    REG16I_QUICK_DEF(0x47, GyrZReg, gyr_z_reg);
    

    struct IntPinCfgReg:public Reg8<>{
        scexpr RegAddress address = 55;

        const uint8_t __resv__:1 = 0;
        uint8_t bypass_en:1 = 0;
        uint8_t fsync_int_mode_en:1;
        uint8_t actl_fsync:1;

        uint8_t int_anyed_2clear:1;
        uint8_t latch_int_en:1;
        uint8_t open:1;
        uint8_t actl:1;

    } int_pin_cfg_reg = {};

    struct WhoAmIReg:public Reg8<>{
        scexpr RegAddress address = 0x75;
        uint8_t data;
    } whoami_reg = {};

    Package package_ = Package::MPU6050;
    real_t acc_scaler = 0;
    real_t gyr_scaler = 0;

    [[nodiscard]] static constexpr 
    uint8_t package2whoami(const Package package){return uint8_t(package);}

    bool data_valid = false;


    // [[nodiscard]] virtual Result<void, BusError> writeReg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] Result<void, BusError> writeReg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> writeReg(const T & reg){return writeReg(reg.address, reg);}

    // [[nodiscard]] virtual Result<void, BusError> readReg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] Result<void, BusError> readReg(const uint8_t addr, uint8_t & data);

    template<typename T>
    [[nodiscard]] Result<void, BusError> readReg(T & reg){return readReg(reg.address, reg);}

    // [[nodiscard]] virtual Result<void, BusError> requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] Result<void, BusError> requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    
    static constexpr real_t calculateAccScale(const AccRange range){
        constexpr double g = 9.806;
        switch(range){
            default:
            case AccRange::_2G:
                return real_t(g * 2);
            case AccRange::_4G:
                return real_t(g * 4);
            case AccRange::_8G:
                return real_t(g * 8);
            case AccRange::_16G:
                return real_t(g * 16);
        }
    }

    static constexpr real_t calculateGyrScale(const GyrRange range){
        switch(range){
            default:
            case GyrRange::_250deg:
                return real_t(ANGLE2RAD(250));
            case GyrRange::_500deg:
                return real_t(ANGLE2RAD(500));
            case GyrRange::_1000deg:
                return real_t(ANGLE2RAD(1000));
            case GyrRange::_2000deg:
                return real_t(ANGLE2RAD(2000));
        }
    }

    MPU6050(const hal::I2cDrv i2c_drv, const Package package);
public:
    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}
    MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}
    MPU6050(hal::I2c & bus, const uint8_t i2c_addr = default_i2c_addr):
        MPU6050(hal::I2cDrv(bus, i2c_addr), Package::MPU6050){;}

    Result<void, Error> verify();

    Result<void, Error> init();
    
    Result<void, Error> update();

    [[nodiscard]] Option<Vector3R> getAcc();
    [[nodiscard]] Option<Vector3R> getGyr();
    [[nodiscard]] Option<real_t> getTemperature();

    Result<void, Error> setAccRange(const AccRange range);
    Result<void, Error> setGyrRange(const GyrRange range);

    Result<void, Error> reset();

    Result<void, Error> setPackage(const Package package){
        package_ = package;
        return Ok();
    }

    Result<Package, Error> getPackage();

    Result<void, Error> enableDirectMode(const Enable en = EN);
};

};

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::MPU6050::Error, BusError> {
        static Result<T, drivers::MPU6050::Error> convert(const BusError berr){
            using Error = drivers::MPU6050::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return MPU6050::Error::I2C_NOT_READY;
                    default: return Error::UNSPECIFIED;
                }
            }(berr);

            return Err(err); 
        }
    };
}