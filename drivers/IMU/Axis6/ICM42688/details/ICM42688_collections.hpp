

#pragma once

#include "drivers/IMU/details/InvensenseIMU.hpp"
#include "core/io/regs.hpp"
#include "core/utils/Reg.hpp"

namespace ymd::drivers{

struct ICM42688_Collections{
    static constexpr auto ICM42688_WHO_AM_I = 0x75;

    //when ad0 is low
    // static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1101000); 

    //when ad0 is high
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b1101001); 

    // static constexpr 
    enum class AccFs:uint8_t{
        _16G,// default
        _8G,
        _4G,
        _2G,
    };

    enum class AccOdr:uint8_t
    {
        _32000HZ = 1,
        _16000HZ,
        _8000HZ,
        _4000HZ,
        _2000HZ,
        _1000HZ,// default
        _200HZ,
        _100HZ,
        _50HZ,
        _25HZ,
        _12_5HZ,
        _6_25HZ,
        _3_125HZ,
        _1_5625HZ,
        _500HZ,
    };


    enum class GyrFs:uint8_t{
        _2000DPS,// default
        _1000DPS,
        _500DPS,
        _250DPS,
        _125DPS,
        _62_5DPS,
        _31_25DPS,
        _15_625DPS,
    };

    enum class GyrOdr:uint8_t{
        _32000HZ,
        _16000HZ,
        _8000HZ,
        _4000HZ,
        _2000HZ,
        _1000HZ,// default
        _200HZ,
        _100HZ,
        _50HZ,
        _25HZ,
        _12_5HZ,
        _X0HZ,
        _X1HZ,
        _X2HZ,
        _500HZ,
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

    using RegAddress = uint8_t;

    enum class Bank{
        _0 = 0,
        _1,
        _2,
        _3,
        _4
    };
};

struct ICM42688_Regs:public ICM42688_Collections{
    struct R8_DEVICE_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x11;
        static constexpr Bank bank = Bank::_0;
        uint8_t soft_reset_config:1;
        uint8_t __resv1__:3;
        uint8_t spi_mode:1;
        uint8_t __resv2__:3;
    }DEF_R8(device_config_reg)

    struct R8_DRIVE_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x13;
        static constexpr Bank bank = Bank::_0;
        uint8_t spi_slew_rate:3;
        uint8_t i2c_slew_rate:3;
        uint8_t :2;
    }DEF_R8(drive_config_reg)

    struct R8_INT_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x14;
        uint8_t int1_polatity:1;
        uint8_t int1_drive_circuit:1;
        uint8_t int1_mode:1;
        uint8_t int2_polatity:1;
        uint8_t int2_drive_circuit:1;
        uint8_t int2_mode:1;
        uint8_t :2;
    }DEF_R8(int_config_reg)

    struct R8_FIFO_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x16;
        uint8_t :6;
        uint8_t fifo_mode:2;
    }DEF_R8(fifo_config_reg)

    struct R16_TEMPDATA:public Reg16<>{
        static constexpr RegAddress address = 0x1D;
        uint16_t data;
    }DEF_R16(tempdata_reg)

    // #pragma pack(push, 1)
    // struct R16_ACCEL_DATA_X:public Reg16<>{
    //     static constexpr RegAddress address = 0x1f;
    //     int16_t data;
    // }DEF_R16(accel_data_x_reg)

    // struct R16_ACCEL_DATA_Y:public Reg16<>{
    //     static constexpr RegAddress address = 0x21;
    //     int16_t data;
    // }DEF_R16(accel_data_y_reg)

    // struct R16_ACCEL_DATA_Z:public Reg16<>{
    //     static constexpr RegAddress address = 0x23;
    //     int16_t data;
    // }DEF_R16(accel_data_z_reg)

    // struct R16_GYRO_DATA_X:public Reg16<>{
    //     static constexpr RegAddress address = 0x25;
    //     int16_t data;
    // }DEF_R16(gyro_data_x_reg)

    // struct R16_GYRO_DATA_Y:public Reg16<>{
    //     static constexpr RegAddress address = 0x27;
    //     int16_t data;
    // }DEF_R16(gyro_data_y_reg)

    // struct R16_GYRO_DATA_Z:public Reg16<>{
    //     static constexpr RegAddress address = 0x29;
    //     int16_t data;
    // }DEF_R16(gyro_data_z_reg)
    // #pragma pack(pop)

    #pragma pack(push, 1)
    Vector3_t<int16_t> acc_data_;
    Vector3_t<int16_t> gyr_data_;
    #pragma pack(pop)

    static_assert(sizeof(acc_data_) == 6);
    static_assert(sizeof(gyr_data_) == 6);
    
    struct R16_TMST_FSYNC:public Reg16<>{
        static constexpr RegAddress address = 0x2B;
        int16_t data;
    }DEF_R16(tmst_fsync_reg)

    struct R8_INT_STATUS1:public Reg8<>{
        static constexpr RegAddress address = 0x2D;
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
        static constexpr RegAddress address = 0x2E;
        uint16_t fifo_count;
    }DEF_R16(fifo_count_reg)

    struct R8_FIFO_DATA:public Reg8<>{
        static constexpr RegAddress address = 0x30;
        uint8_t fifo_data;
    }DEF_R8(fifo_data_reg)

    struct R16_APEX_DATA0:public Reg16<>{
        static constexpr RegAddress address = 0x31;
        // uint8_t data;
        uint16_t step_cnt;
    }DEF_R16(apex_data0_reg)

    struct R8_APEX_DATA2:public Reg8<>{
        static constexpr RegAddress address = 0x33;
        uint8_t step_cadence;
    }DEF_R8(apex_data2_reg)

    struct R8_APEX_DATA3:public Reg8<>{
        static constexpr RegAddress address = 0x34;
        uint8_t activity_class:2;
        uint8_t dmp_idle:1;
        uint8_t :5;
    }DEF_R8(apex_data3_reg)

    struct R8_APEX_DATA4:public Reg8<>{
        static constexpr RegAddress address = 0x35;
        uint8_t tap_dir:1;
        uint8_t tap_axis:2;
        uint8_t tap_num:2;
        uint8_t :3;
    }DEF_R8(apex_data4_reg)

    struct R8_APEX_DATA5:public Reg8<>{
        static constexpr RegAddress address = 0x36;
        uint8_t double_tap_timing:6;
        uint8_t :2;
    }DEF_R8(apex_data5_reg)

    struct R8_INT_STATUS2:public Reg8<>{
        static constexpr RegAddress address = 0x37;
        uint8_t wom_x_int:1;
        uint8_t wom_y_int:1;
        uint8_t wom_z_int:1;
        uint8_t smd_int:1;
        uint8_t :4;
    }DEF_R8(int_status2_reg)

    struct R8_INT_STATUS3:public Reg8<>{
        static constexpr RegAddress address = 0x38;
        uint8_t tap_det_int:1;
        uint8_t sleep_int:1;
        uint8_t wake_int:1;
        uint8_t tilt_det_int:1;
        uint8_t step_cnt_ovf_int:1;
        uint8_t step_det_int:1;
        uint8_t :2;
    }DEF_R8(int_status3_reg)

    struct R8_GYRO_CONFIG0:public Reg8<>{ 
        static constexpr RegAddress address = 0x4f;
        static constexpr Bank bank = Bank::_0;
        GyrOdr gyro_odr:4;
        const uint8_t __resv__:1 = 0;
        GyrFs gyro_fs:3;
    }DEF_R8(gyro_config0_reg)

    struct R8_ACCEL_CONFIG0:public Reg8<>{
        static constexpr RegAddress address = 0x50;
        static constexpr Bank bank = Bank::_0;
        AccOdr accel_odr:4;
        const uint8_t __resv__:1 = 0;
        AccFs accel_fs:3;
    }DEF_R8(accel_config0_reg)

    // struct R8_GYRO_CONFIG1:public Reg8<>{
    //     static constexpr RegAddress address = 0x51;
    //     static constexpr Bank bank = Bank::_0;
    //     GyrOdr gyro_odr:4;
    //     const uint8_t __resv__:1 = 0;
    //     GyrFs gyro_fs:3;
    // }DEF_R8(gyro_config1_reg)

    struct R8_WHO_AM_I:public Reg8<>{
        static constexpr RegAddress address = 0x75;
        static constexpr Bank bank = Bank::_0;

        static constexpr uint8_t KEY = 0x47;
        uint8_t data;
    }DEF_R8(who_am_i_reg)
};

}