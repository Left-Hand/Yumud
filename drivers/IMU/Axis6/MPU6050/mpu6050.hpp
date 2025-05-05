#pragma once

//这个驱动已经完成了基础使用

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "drivers/IMU/IMU.hpp"

#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

class MPU6050:public AccelerometerIntf, public GyroscopeIntf{
public:
    using Error = ImuError;
    
    enum class Package:uint8_t{
        MPU6050 = 0x68,
        MPU6500 = 0x70,
        MPU9250 = 0x71
    };


    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u8(0b11010000);

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
    using Phy = InvensenseSensor_Phy;

    using RegAddress = uint8_t;    

    Phy phy_;

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
    real_t acc_scaler_ = 0;
    real_t gyr_scaler_ = 0;

    [[nodiscard]] static constexpr 
    uint8_t package2whoami(const Package package){return uint8_t(package);}

    bool data_valid = false;

    static constexpr real_t calculate_acc_scale(const AccRange range){
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

    static constexpr real_t calculate_gyr_scale(const GyrRange range){
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

    [[nodiscard]] Result<void, Error> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] Result<void, Error> write_reg(const T & reg){
        return write_reg(reg.address, reg);
    }

    [[nodiscard]] Result<void, Error> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] Result<void, Error> read_burst(const uint8_t addr, int16_t * data, const size_t len){
        return phy_.read_burst(addr, data, len);
    }

    template<typename T>
    [[nodiscard]] Result<void, Error> read_reg(T & reg){
        return read_reg(reg.address, reg);
    }
public:
    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}
    MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}
    MPU6050(hal::I2c & bus, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MPU6050(hal::I2cDrv(bus, addr), Package::MPU6050){;}

    [[nodiscard]] Result<void, Error> validate();

    [[nodiscard]] Result<void, Error> init();
    
    [[nodiscard]] Result<void, Error> update();

    [[nodiscard]] Option<Vector3_t<real_t>> get_acc();
    [[nodiscard]] Option<Vector3_t<real_t>> get_gyr();
    [[nodiscard]] Option<real_t> get_temperature();

    [[nodiscard]] Result<void, Error> set_acc_range(const AccRange range);
    [[nodiscard]] Result<void, Error> set_gyr_range(const GyrRange range);

    [[nodiscard]] Result<void, Error> reset();

    [[nodiscard]] Result<void, Error> set_package(const Package package){
        package_ = package;
        return Ok();
    }

    [[nodiscard]] Result<Package, Error> get_package();

    [[nodiscard]] Result<void, Error> enable_direct_mode(const Enable en = EN);
};

};