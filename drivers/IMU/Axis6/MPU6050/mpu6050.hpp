#pragma once

//这个驱动已经完成了基础使用

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"
#include "drivers/IMU/IMU.hpp"

#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

struct MPU6050_Collections{
    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;

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

    using RegAddress = uint8_t;   
};

struct MPU6050_Regs:public MPU6050_Collections{ 
    struct GyrConfReg:public Reg8<>{
        scexpr RegAddress address = 0x1b;

        const uint8_t __resv__:3 = 0;
        GyrRange fs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    } gyr_conf_reg = {};
    

    struct AccConfReg:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        const uint8_t __resv__:3 = 0;
        AccRange afs_sel:2;
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
};

class MPU6050:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public MPU6050_Regs{
public:
    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}
    MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}
    MPU6050(hal::I2c & bus, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MPU6050(hal::I2cDrv(bus, addr), Package::MPU6050){;}

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> init();
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vector3<q24>> read_acc();
    [[nodiscard]] IResult<Vector3<q24>> read_gyr();
    [[nodiscard]] IResult<real_t> read_temp();

    [[nodiscard]] IResult<> set_acc_range(const AccRange range);
    [[nodiscard]] IResult<> set_gyr_range(const GyrRange range);

    [[nodiscard]] IResult<> reset();

    void set_package(const Package package){
        package_ = package;
    }

    [[nodiscard]] Result<Package, Error> get_package();

    [[nodiscard]] IResult<> enable_direct_mode(const Enable en = EN);
private:

    using Phy = InvensenseSensor_Phy;
    Phy phy_;

    bool data_valid = false;
    Package package_ = Package::MPU6050;
    real_t acc_scaler_ = 0;
    real_t gyr_scaler_ = 0;


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

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<int16_t> pdata){
        return phy_.read_burst(addr, pdata.data(), pdata.size());
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(reg.address, reg.as_ref());
    }

    [[nodiscard]] static constexpr 
    uint8_t package2whoami(const Package package){return uint8_t(package);}
};

};