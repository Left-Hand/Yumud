#pragma once

//这个驱动已经完成了基础使用

#include "core/io/regs.hpp"
#include "core/utils/Errno.hpp"

#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/InvensenseIMU.hpp"

namespace ymd::drivers{

struct MPU6050_Prelude{
    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Package:uint8_t{
        MPU6050 = 0x68,
        MPU6500 = 0x70,
        MPU9250 = 0x71
    };


    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1101000);


    enum class AccFs:uint8_t{
        _2G     =   0,
        _4G     =   1,
        _8G     =   2,
        _16G    =   3
    };

    enum class GyrFs:uint8_t{
        _250deg     =   0,
        _500deg     =   1,
        _1000deg    =   2,
        _2000deg    =   3
    };

    using RegAddress = uint8_t;   

    struct Config{
        Package packge = Package::MPU6050;
        AccFs acc_fs = AccFs::_2G;
        GyrFs gyr_fs = GyrFs::_1000deg;
    };
};

struct MPU6050_Regs:public MPU6050_Prelude{ 
    struct GyrConfReg:public Reg8<>{
        static constexpr RegAddress address = 0x1b;

        const uint8_t __resv__:3 = 0;
        GyrFs fs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    }DEF_R8(gyr_conf_reg)
    

    struct AccConfReg:public Reg8<>{
        static constexpr RegAddress address = 0x1c;

        const uint8_t __resv__:3 = 0;
        AccFs afs_sel:2;
        uint8_t zg_st:1 = 0;
        uint8_t yg_st:1 = 0;
        uint8_t xg_st:1 = 0;
    }DEF_R8(acc_conf_reg)
    
    REG16I_QUICK_DEF(0x3B, AccXReg, acc_x_reg);
    REG16I_QUICK_DEF(0x3D, AccYReg, acc_y_reg);
    REG16I_QUICK_DEF(0x3F, AccZReg, acc_z_reg);

    REG16_QUICK_DEF(0x41, TemperatureReg, temperature_reg);

    REG16I_QUICK_DEF(0x43, GyrXReg, gyr_x_reg);
    REG16I_QUICK_DEF(0x45, GyrYReg, gyr_y_reg);
    REG16I_QUICK_DEF(0x47, GyrZReg, gyr_z_reg);
    

    struct IntPinCfgReg:public Reg8<>{
        static constexpr RegAddress address = 55;

        const uint8_t __resv__:1 = 0;
        uint8_t bypass_en:1 = 0;
        uint8_t fsync_int_mode_en:1;
        uint8_t actl_fsync:1;

        uint8_t int_anyed_2clear:1;
        uint8_t latch_int_en:1;
        uint8_t open:1;
        uint8_t actl:1;

    }DEF_R8(int_pin_cfg_reg)

    struct WhoAmIReg:public Reg8<>{
        static constexpr RegAddress address = 0x75;
        uint8_t data;
    } DEF_R8(whoami_reg)
};
}