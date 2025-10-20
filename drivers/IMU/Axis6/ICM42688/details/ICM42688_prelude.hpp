

#pragma once

#include "drivers/IMU/details/InvensenseIMU.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct ICM42688_Prelude{
    static constexpr auto ICM42688_WHO_AM_I = 0x75;

    struct I2cSlaveAddrMaker{
        BoolLevel ad0_level;

        constexpr hal::I2cSlaveAddr<7> to_i2c_addr() const {
            return hal::I2cSlaveAddr<7>::from_u7(0b1101000 | (ad0_level == HIGH));
        }
    };

    enum class AccFs:uint8_t{
        _16G,// default
        _8G,
        _4G,
        _2G,
    };

    enum class AccOdr:uint8_t{
        _32000Hz = 1,
        _16000Hz,
        _8000Hz,
        _4000Hz,
        _2000Hz,
        _1000Hz,// default
        _200Hz,
        _100Hz,
        _50Hz,
        _25Hz,
        _12_5Hz,
        _6_25Hz,
        _3_125Hz,
        _1_5625Hz,
        _500Hz,
    };


    enum class GyrFs:uint8_t{
        _2000deg,// default
        _1000deg,
        _500deg,
        _250deg,
        _125deg,
        _62_5deg,
        _31_25deg,
        _15_625deg,
    };

    enum class GyrOdr:uint8_t{
        _32000Hz,
        _16000Hz,
        _8000Hz,
        _4000Hz,
        _2000Hz,
        _1000Hz,// default
        _200Hz,
        _100Hz,
        _50Hz,
        _25Hz,
        _12_5Hz,
        _X0Hz,
        _X1Hz,
        _X2Hz,
        _500Hz,
    };

    struct Config{
        AccOdr acc_odr = AccOdr::_1000Hz;
        AccFs acc_fs = AccFs::_8G;
        GyrOdr gyr_odr = GyrOdr::_1000Hz;
        GyrFs gyr_fs = GyrFs::_1000deg;
    };

    enum class TempFiltBw:uint8_t{
        _4000Hz = 0,
        _170Hz,
        _82Hz,
        _40Hz,
        _20Hz,
        _10Hz,
        _5Hz = 6
    };

    struct Coeff{
        enum class GyrUiFilterOrder{
            _1,
            _2,
            _3,
        };

        enum class DEC2_M2_Order{
            _3 = 0b10
        };

        enum class AccUiFilterBandwidth{
            //TODO
            
            
        };

        enum class GyroUiFIlterBandwidth{
            //TODO
            
            
        };
    };
    
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddr = uint8_t;

    enum class Bank{
        _0 = 0,
        _1,
        _2,
        _3,
        _4
    };
};

struct ICM42688_Regset:public ICM42688_Prelude{
    struct R8_DEVICE_CONFIG:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x11;
        static constexpr Bank bank = Bank::_0;
        uint8_t soft_reset_config:1;
        uint8_t __resv1__:3;
        uint8_t spi_mode:1;
        uint8_t __resv2__:3;
    }DEF_R8(device_config_reg)

    struct R8_DRIVE_CONFIG:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x13;
        static constexpr Bank bank = Bank::_0;
        uint8_t spi_slew_rate:3;
        uint8_t i2c_slew_rate:3;
        uint8_t :2;
    }DEF_R8(drive_config_reg)

    struct R8_INT_CONFIG:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x14;
        uint8_t int1_polatity:1;
        uint8_t int1_drive_circuit:1;
        uint8_t int1_mode:1;
        uint8_t int2_polatity:1;
        uint8_t int2_drive_circuit:1;
        uint8_t int2_mode:1;
        uint8_t :2;
    }DEF_R8(int_config_reg)

    struct R8_FIFO_CONFIG:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x16;
        uint8_t :6;
        uint8_t fifo_mode:2;
    }DEF_R8(fifo_config_reg)

    struct R16_TEMpbuf:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x1D;
        uint16_t data;
    }DEF_R16(tempbuf_reg)

    #pragma pack(push, 1)
    Vec3<int16_t> acc_data_ = Vec3<int16_t>::ZERO;
    Vec3<int16_t> gyr_data_ = Vec3<int16_t>::ZERO;
    #pragma pack(pop)
    
    struct R16_TMST_FSYNC:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x2B;
        int16_t data;
    }DEF_R16(tmst_fsync_reg)

    struct R8_INT_STATUS1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x2D;
        uint8_t agc_rdy_int:1;
        uint8_t fifo_full_int:1;
        uint8_t fifo_ths_int:1;
        uint8_t data_rdy_int:1;

        uint8_t reset_done_int:1;
        uint8_t pll_rdy_int:1;
        uint8_t ui_fsync_int:1;
        uint8_t :1;
    }DEF_R8(int_status1_reg)

    struct R16_FIFO_COUNT:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x2E;
        uint16_t fifo_count;
    }DEF_R16(fifo_count_reg)

    struct R8_FIFO_DATA:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x30;
        uint8_t fifo_data;
    }DEF_R8(fifo_data_reg)

    struct R16_APEX_DATA0:public Reg16<>{
        static constexpr RegAddr ADDRESS = 0x31;
        // uint8_t data;
        uint16_t step_cnt;
    }DEF_R16(apex_data0_reg)

    struct R8_APEX_DATA2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x33;
        uint8_t step_cadence;
    }DEF_R8(apex_data2_reg)

    struct R8_APEX_DATA3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x34;
        uint8_t activity_class:2;
        uint8_t dmp_idle:1;
        uint8_t :5;
    }DEF_R8(apex_data3_reg)

    struct R8_APEX_DATA4:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x35;
        uint8_t tap_dir:1;
        uint8_t tap_axis:2;
        uint8_t tap_num:2;
        uint8_t :3;
    }DEF_R8(apex_data4_reg)

    struct R8_APEX_DATA5:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x36;
        uint8_t double_tap_timing:6;
        uint8_t :2;
    }DEF_R8(apex_data5_reg)

    struct R8_INT_STATUS2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x37;
        uint8_t wom_x_int:1;
        uint8_t wom_y_int:1;
        uint8_t wom_z_int:1;
        uint8_t smd_int:1;
        uint8_t :4;
    }DEF_R8(int_status2_reg)

    struct R8_INT_STATUS3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x38;
        uint8_t tap_det_int:1;
        uint8_t sleep_int:1;
        uint8_t wake_int:1;
        uint8_t tilt_det_int:1;
        uint8_t step_cnt_ovf_int:1;
        uint8_t step_det_int:1;
        uint8_t :2;
    }DEF_R8(int_status3_reg)

    struct R8_GYRO_CONFIG0:public Reg8<>{ 
        static constexpr RegAddr ADDRESS = 0x4f;
        static constexpr Bank bank = Bank::_0;
        GyrOdr gyro_odr:4;
        const uint8_t __resv__:1 = 0;
        GyrFs gyro_fs:3;
    }DEF_R8(gyro_config0_reg)

    struct R8_ACCEL_CONFIG0:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x50;
        static constexpr Bank bank = Bank::_0;
        AccOdr accel_odr:4;
        const uint8_t __resv__:1 = 0;
        AccFs accel_fs:3;
    }DEF_R8(accel_config0_reg)

    // struct R8_GYRO_CONFIG1:public Reg8<>{
    //     static constexpr RegAddr ADDRESS = 0x51;
    //     static constexpr Bank bank = Bank::_0;
    //     GyrOdr gyro_odr:4;
    //     const uint8_t __resv__:1 = 0;
    //     GyrFs gyro_fs:3;
    // }DEF_R8(gyro_config1_reg)

    struct R8_WHO_AM_I:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x75;
        static constexpr Bank bank = Bank::_0;

        static constexpr uint8_t KEY = 0x47;
        uint8_t data;
    }DEF_R8(who_am_i_reg)
};

}