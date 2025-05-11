

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
    enum class AFS:uint8_t{
        _16G,// default
        _8G,
        _4G,
        _2G,
    };

    enum class AODR:uint8_t
    {
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
        _6_25HZ,
        _3_125HZ,
        _1_5625HZ,
        _500HZ,
    };

    enum class GFS:uint8_t{
        _2000DPS,// default
        _1000DPS,
        _500DPS,
        _250DPS,
        _125DPS,
        _62_5DPS,
        _31_25DPS,
        _15_625DPS,
    };

    enum class GODR:uint8_t{
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

    // enum class RegAddress:uint8_t{
    //     ID = 0x47,
    //     DEVICE_CONFIG             = 0x11,
    //     DRIVE_CONFIG              = 0x13,
    //     INT_CONFIG                = 0x14,
    //     FIFO_CONFIG               = 0x16,
    //     TEMP_DATA1                = 0x1D,
    //     TEMP_DATA0                = 0x1E,
    //     ACC_DATA_X1             = 0x1F,
    //     ACC_DATA_X0             = 0x20,
    //     ACC_DATA_Y1             = 0x21,
    //     ACC_DATA_Y0             = 0x22,
    //     ACC_DATA_Z1             = 0x23,
    //     ACC_DATA_Z0             = 0x24,
    //     GYR_DATA_X1              = 0x25,
    //     GYR_DATA_X0              = 0x26,
    //     GYR_DATA_Y1              = 0x27,
    //     GYR_DATA_Y0              = 0x28,
    //     GYR_DATA_Z1              = 0x29,
    //     GYR_DATA_Z0              = 0x2A,
    //     TMST_FSYNCH               = 0x2B,
    //     TMST_FSYNCL               = 0x2C,
    //     INT_STATUS                = 0x2D,
    //     FIFO_COUNTH               = 0x2E,
    //     FIFO_COUNTL               = 0x2F,
    //     FIFO_DATA                 = 0x30,
    //     APEX_DATA0                = 0x31,
    //     APEX_DATA1                = 0x32,
    //     APEX_DATA2                = 0x33,
    //     APEX_DATA3                = 0x34,
    //     APEX_DATA4                = 0x35,
    //     APEX_DATA5                = 0x36,
    //     INT_STATUS2               = 0x37,
    //     INT_STATUS3               = 0x38,
    //     SIGNAL_PATH_RESET         = 0x4B,
    //     INTF_CONFIG0              = 0x4C,
    //     INTF_CONFIG1              = 0x4D,
    //     PWR_MGMT0                 = 0x4E,
    //     GYR_CONFIG0              = 0x4F,
    //     ACC_CONFIG0             = 0x50,
    //     GYR_CONFIG1              = 0x51,
    //     GYR_ACC_CONFIG0        = 0x52,
    //     ACC_CONFIG1             = 0x53,
    //     TMST_CONFIG               = 0x54,
    //     APEX_CONFIG0              = 0x56,
    //     SMD_CONFIG                = 0x57,
    //     FIFO_CONFIG1              = 0x5F,
    //     FIFO_CONFIG2              = 0x60,
    //     FIFO_CONFIG3              = 0x61,
    //     FSYNC_CONFIG              = 0x62,
    //     INT_CONFIG0               = 0x63,
    //     INT_CONFIG1               = 0x64,
    //     INT_SOURCE0               = 0x65,
    //     INT_SOURCE1               = 0x66,
    //     INT_SOURCE3               = 0x68,
    //     INT_SOURCE4               = 0x69,
    //     FIFO_LOST_PKT0            = 0x6C,
    //     FIFO_LOST_PKT1            = 0x6D,
    //     SELF_TEST_CONFIG          = 0x70,
    //     WHO_AM_I                  = 0x75,
    //     REG_BANK_SEL              = 0x76,
    //     SENSOR_CONFIG0            = 0x03,
    //     GYR_CONFIG_STATIC2       = 0x0B,
    //     GYR_CONFIG_STATIC3       = 0x0C,
    //     GYR_CONFIG_STATIC4       = 0x0D,
    //     GYR_CONFIG_STATIC5       = 0x0E,
    //     GYR_CONFIG_STATIC6       = 0x0F,
    //     GYR_CONFIG_STATIC7       = 0x10,
    //     GYR_CONFIG_STATIC8       = 0x11,
    //     GYR_CONFIG_STATIC9       = 0x12,
    //     GYR_CONFIG_STATIC10      = 0x13,
    //     XG_ST_DATA                = 0x5F,
    //     YG_ST_DATA                = 0x60,
    //     ZG_ST_DATA                = 0x61,
    //     TMSTVAL0                  = 0x62,
    //     TMSTVAL1                  = 0x63,
    //     TMSTVAL2                  = 0x64,
    //     INTF_CONFIG4              = 0x7A,
    //     INTF_CONFIG5              = 0x7B,
    //     INTF_CONFIG6              = 0x7C
    // };
    
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;
};

struct ICM42688_Regs:public ICM42688_Collections{
    struct R8_DEVICE_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x11;
        uint8_t soft_reset_config:1;
        uint8_t __resv1__:3;
        uint8_t spi_mode:1;
        uint8_t __resv2__:3;
    }DEF_R8(device_config_reg);

    struct R8_DRIVE_CONFIG:public Reg8<>{
        static constexpr RegAddress address = 0x13;
        uint8_t spi_slew_rate:3;
        uint8_t i2c_slew_rate:3;
        uint8_t :2;
    }DEF_R8(drive_config_reg);

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

    struct R16_ACCEL_DATA_X:public Reg16<>{
        static constexpr RegAddress address = 0x1f;
        int16_t data;
    }DEF_R16(accel_data_x_reg)
    struct R16_ACCEL_DATA_Y:public Reg16<>{
        static constexpr RegAddress address = 0x21;
        int16_t data;
    }DEF_R16(accel_data_y_reg)
    struct R16_ACCEL_DATA_Z:public Reg16<>{
        static constexpr RegAddress address = 0x23;
        int16_t data;
    }DEF_R16(accel_data_z_reg)

    struct R16_GYRO_DATA_X:public Reg16<>{
        static constexpr RegAddress address = 0x25;
        int16_t data;
    }DEF_R16(gyro_data_x_reg)
    struct R16_GYRO_DATA_Y:public Reg16<>{
        static constexpr RegAddress address = 0x27;
        int16_t data;
    }DEF_R16(gyro_data_y_reg)
    struct R16_GYRO_DATA_Z:public Reg16<>{
        static constexpr RegAddress address = 0x29;
        int16_t data;
    }DEF_R16(gyro_data_z_reg)

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

};

}