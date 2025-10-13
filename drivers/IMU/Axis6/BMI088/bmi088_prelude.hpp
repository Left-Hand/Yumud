#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/IMU.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

struct BMI088_Prelude{
public:
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0xd0 >> 1);

    using Error = ImuError;
    
    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class AccFs:uint8_t{
        _3G,
        _6G,
        _12G,
        _24G
    };

    enum class GyrFs:uint8_t{
        _2000deg,
        _1000deg,
        _500deg ,
        _250deg ,
        _125deg ,
    };

    enum class AccBwp:uint8_t{
        _4Fold = 0x08,
        _2Fold = 0x09,
        Normal = 0x0A,
    };

    enum class AccOdr:uint8_t{
        _12_5Hz = 0x05,
        _25Hz ,
        _50Hz ,
        _100Hz,
        _200Hz,
        _400Hz,
        _800Hz,
        _1600Hz
    };

    enum class GyrOdr:uint8_t{
        _2000Hz_High = 0x00,
        _2000Hz = 0x01,
        _1000Hz,
        _400Hz,
        _200Hz,
        _100Hz,
    };

    using RegAddr = uint8_t;
};

struct BMI088_AccRegs:public BMI088_Prelude{

    REG8_QUICK_DEF(0x01, AccChipId, acc_chipid_reg);

    struct R8_AccErr:public Reg8<>{
        static constexpr RegAddr address = 0x02;

        uint8_t fatal_err:1;
        const uint8_t __resv__:1;
        uint8_t err_code:3;
        uint8_t xg_st:3;
    }DEF_R8(acc_err_reg)

    struct R8_AccStatus:public Reg8<>{
        static constexpr RegAddr address = 0x03;
        const uint8_t __resv__:7;
        uint8_t drdy_acc:1 = 0;
    }DEF_R8(acc_stat_reg)

    REG16I_QUICK_DEF(0x12, R8_AccXReg, acc_x_reg);
    REG16I_QUICK_DEF(0x14, R8_AccYReg, acc_y_reg);
    REG16I_QUICK_DEF(0x16, R8_AccZReg, acc_z_reg);
    REG8_QUICK_DEF(0x18,  R8_SensorTime0, sensor_t0_reg);
    REG8_QUICK_DEF(0x19,  R8_SensorTime1, sensor_t1_reg);
    REG8_QUICK_DEF(0x1A,  R8_SensorTime2, sensor_t2_reg);

    struct R8_AccIntStatus1:public Reg8<>{
        static constexpr RegAddr address = 0x1D;
        const uint8_t __resv__:7 = 0;
        uint8_t acc_drdy:1 = 0;
    }DEF_R8(acc_intstat1_reg)

    REG8_QUICK_DEF(0x22, Temperature, temp_reg);
    struct R8_AccConf:public Reg8<>{
        static constexpr RegAddr address = 0x40;

        uint8_t acc_odr:4;
        uint8_t acc_bwp:4;
    }DEF_R8(acc_conf_reg)

    struct R8_AccFs:public Reg8<>{
        static constexpr RegAddr address = 0x41;
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
        static constexpr RegAddr address = 0x53;
    }DEF_R8(int1_ctrl_reg)

    struct R8_Int2Ctrl:public _R8_IoCtrl{
        static constexpr RegAddr address = 0x54;
    }DEF_R8(int2_ctrl_reg)

    struct R8_Int1Mapbuf:public Reg8<>{
        static constexpr RegAddr address = 0x56;
        uint8_t __resv1__:2;
        uint8_t int1_drdy:1;
        uint8_t __resv2__:3;
        uint8_t int2_drdy:1;
        uint8_t __resv3__:1;
    }DEF_R8(int1_map_data_reg)

    REG8_QUICK_DEF(0x6D, R8_AccSelfTest, acc_selftest_reg);
    REG8_QUICK_DEF(0x7c, R8_AccPwrConf, acc_pwrconf_reg);
    REG8_QUICK_DEF(0x7d, R8_AccPwrCtrl, acc_pwrctrl_reg);
};

struct BMI088_GyrRegs:public BMI088_Prelude{
    REG8_QUICK_DEF(0x00, R8_GyroChipID, gyro_chip_id);
    REG16I_QUICK_DEF(0x02, R8_AccXReg, gyr_x_reg);
    REG16I_QUICK_DEF(0x04, R8_AccYReg, gyr_y_reg);
    REG16I_QUICK_DEF(0x06, R8_AccZReg, gyr_z_reg);
    
    struct R8_GyroIntStatus1:public Reg8<>{
        static constexpr RegAddr address = 0x0A;
        const uint8_t __resv__:7 = 0;
        uint8_t gyro_drdy:1 = 0;
    }DEF_R8(gyro_intstat1_reg)

    REG8_QUICK_DEF(0x0f, R8_GyroRange, gyro_range_reg);
    REG8_QUICK_DEF(0x10, R8_GyroBw, gyro_bw_reg);
    REG8_QUICK_DEF(0x11, R8_GyroPm, gyro_pm_reg);
    REG8_QUICK_DEF(0x14, R8_SoftReset, soft_reset_reg);
    REG8_QUICK_DEF(0x15, R8_GyroIntCtrl, gyro_intctrl_reg);

    struct R8_Int3Int4Conf:public Reg8<>{
        static constexpr RegAddr address = 0x16;
        uint8_t int3_lvl:1;
        uint8_t int3_od:1;
        uint8_t int4_lvl:1;
        uint8_t int4_od:1;
        uint8_t __resv__:4;
    }DEF_R8(int34_conf_reg)

    REG8_QUICK_DEF(0x18, R8_Int3Int4IoMap, int34_iomap_reg);

    struct R8_GyroSelfTest:public Reg8<>{
        static constexpr RegAddr address = 0x3c;
        uint8_t trig_bist:1;
        uint8_t bist_rdy:1;
        uint8_t bist_fail:1;
        uint8_t :1;
        uint8_t rate_ok:1;
        uint8_t :3;
    }DEF_R8(gyro_selftest_reg)
};

}