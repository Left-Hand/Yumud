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

    using DeviceResult = Result<void, BusError>;
    __inline DeviceResult make_result(const BusError res){
        if(res.ok()) return Ok();
        else return Err(res); 
    }


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
    
    Package package_ = Package::MPU6050;
    real_t acc_scaler = 0;
    real_t gyr_scaler = 0;

    [[nodiscard]] static constexpr 
    uint8_t package2whoami(const Package package){return uint8_t(package);}

    bool data_valid = false;


    // [[nodiscard]] virtual DeviceResult writeReg(const uint8_t addr, const uint8_t data);
    [[nodiscard]] DeviceResult writeReg(const uint8_t addr, const uint8_t data);

    // [[nodiscard]] virtual DeviceResult readReg(const uint8_t addr, uint8_t & data);
    [[nodiscard]] DeviceResult readReg(const uint8_t addr, uint8_t & data);

    // [[nodiscard]] virtual DeviceResult requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    [[nodiscard]] DeviceResult requestData(const uint8_t reg_addr, int16_t * datas, const size_t len);
    
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

    MPU6050(const hal::I2cDrv & i2c_drv, const Package package):
        p_i2c_drv_(i2c_drv),
        package_(package){;}

public:
    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}
    MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}
    MPU6050(hal::I2c & bus, const uint8_t i2c_addr = default_i2c_addr):
        MPU6050(hal::I2cDrv(bus, i2c_addr), Package::MPU6050){;}

    bool verify();

    void init();
    
    void update();

    [[nodiscard]] std::tuple<real_t, real_t, real_t> getAcc() override;
    [[nodiscard]] std::tuple<real_t, real_t, real_t> getGyr() override;

    [[nodiscard]] Option<std::tuple<real_t, real_t, real_t>> getAcc2(){
        return Some{getAcc()};
    }

    [[nodiscard]] Option<std::tuple<real_t, real_t, real_t>> getGyr2(){
        return Some{getGyr()};
    }


    [[nodiscard]] Option<real_t> getTemperature();

    void setAccRange(const AccRange range);
    void setGyrRange(const GyrRange range);
};


};