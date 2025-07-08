#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

// https://github.com/hanyazou/BMI160-Arduino/blob/master/BMI160.cpp#L88

namespace ymd::drivers{

struct BMI160_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;



    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
    };

    //  * <pre>
    // * ODR     | 3dB cut-off
    // * --------+--------------
    // *  12.5Hz |  5.06Hz
    // *    25Hz | 10.12Hz
    // *    50Hz | 20.25Hz
    // *   100Hz | 40.5Hz
    // *   200Hz | 80Hz
    // *   400Hz | 162Hz (155Hz for Z axis)
    // *   800Hz | 324Hz (262Hz for Z axis)
    // *  1600Hz | 684Hz (353Hz for Z axis)
    // * </pre>
    enum class AccOdr:uint8_t{

        _25_32Hz = 0b0001,
        _25_16Hz,
        _25_8Hz,
        _25_4Hz,
        _25_2Hz,
        _25Hz,
        
        _50Hz,
        _100Hz,
        _200Hz,
        _400Hz,
        _800Hz,
        _1600Hz
    };

    // *
    // * <pre>
    // * ODR     | 3dB cut-off
    // * --------+------------
    // *    25Hz | 10.7Hz
    // *    50Hz | 20.8Hz
    // *   100Hz | 39.9Hz
    // *   200Hz | 74.6Hz
    // *   400Hz | 136.6Hz
    // *   800Hz | 254.6Hz
    // *  1600Hz | 523.9Hz
    // *  3200Hz | 890Hz
    // * </pre>
    enum class GyrOdr:uint8_t{
        _25Hz = 0b0110,
        
        _50Hz,
        _100Hz,
        _200Hz,
        _400Hz,
        _800Hz,
        _1600Hz,
        _3200Hz
    };
    
    enum class AccFs:uint8_t{
        _2G     =   0b0011,
        _4G     =   0b0101,
        _8G     =   0b1000,
        _16G    =   0b1100
    };

    enum class GyrFs:uint8_t{
        _2000deg = 0b0000,
        _1000deg,
        _500deg,
        _250deg,
        _125deg
    };

    enum class Command:uint8_t{
        START_FOC = 0x04,
        ACC_SET_PMU = 0b0001'0000,
        GYR_SET_PMU = 0b0001'0100,
        MAG_SET_PMU = 0b0001'1000,
        FIFO_FLUSH = 0xB0,
        RESET_INTERRUPT =0xB1,
        SOFT_RESET = 0xB1,
        STEP_CNT_CLR = 0xB2
    };

    enum class PmuType:uint8_t{
        ACC,
        GYR,
        MAG
    };

    enum class PmuMode:uint8_t{
        SUSPEND         = 0b00,
        NORMAL          = 0b01,
        LOW_POWER       = 0b10,
        FAST_SETUP      = 0b11
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0b11010010);

    struct Config{
        AccOdr acc_odr = AccOdr::_800Hz;
        AccFs acc_fs = AccFs::_8G;
        GyrOdr gyr_odr = GyrOdr::_800Hz;
        GyrFs gyr_fs = GyrFs::_1000deg;
    };
};


struct BMI160_Regs:public BMI160_Prelude{

    using RegAddress = uint8_t;

    struct R8_ChipId:public Reg8<>{
        static constexpr RegAddress address = 0x00;
        static constexpr uint8_t CORRECT_ID = 0xD1;
        uint8_t data;
    }DEF_R8(chip_id)

    struct R8_Err:public Reg8<>{
        static constexpr RegAddress address = 0x02;
        uint8_t fatal_err:1;
        uint8_t err_code:4;
        uint8_t i2c_fail_err:1;
        uint8_t drop_cmd_err:1;
        uint8_t mag_drdy_err:1;
    }DEF_R8(err)

    struct R8_PmuStatus:public Reg8<>{
        static constexpr RegAddress address = 0x03;
        PmuMode mag_pmu_status:2;
        PmuMode gyr_pmu_status:2;
        PmuMode acc_pmu_status:2;
        uint8_t:2;
    }DEF_R8(pmu_status)

    struct R8_Rhall:public Reg8<>{
        static constexpr RegAddress address = 0x0A;
    };

    static constexpr RegAddress GYR_ADDRESS = 0x0c;

    // static constexpr RegAddress mag_address = 0x04;
    // static constexpr RegAddress mag_x_address = 0x04;
    // static constexpr RegAddress mag_y_address = 0x06;
    // static constexpr RegAddress mag_z_address = 0x08;
    

    // static constexpr RegAddress gyr_x_address = 0x0c;
    // static constexpr RegAddress gyr_y_address = 0x0e;
    // static constexpr RegAddress gyr_z_address = 0x10;

    // static constexpr RegAddress acc_address = 0x12;
    // static constexpr RegAddress acc_x_address = 0x12;
    // static constexpr RegAddress acc_y_address = 0x14;
    // static constexpr RegAddress acc_z_address = 0x16;

    struct R8_SensorTime:public Reg8<>{
        static constexpr RegAddress address = 0x18; // SENSOR_TIME_2
        uint32_t time: 24; // Assuming 8 bits for sensor time
    };

    struct R8_Status:public Reg8<>{
        static constexpr RegAddress address = 0x1B;
        uint8_t __resv__:1;
        uint8_t gyr_self_test_ok:1;
        uint8_t mag_man_op:1;
        uint8_t foc_rdy:1;
        uint8_t nvm_rdy:1;
        uint8_t drdy_mag: 1;
        uint8_t drdy_gyr: 1;
        uint8_t drdy_acc: 1;
    }DEF_R8(status)

    struct R8_IntStatus0:public Reg8<>{
        static constexpr RegAddress address = 0x1c; // INT_STATUS_3
        uint8_t step_int:1;
        uint8_t sigmot_int:1;
        uint8_t anym_int:1;
        uint8_t pmu_trigger_int:1;
        uint8_t d_tap_int:1;
        uint8_t s_tap_int:1;
        uint8_t orient_int:1;
        uint8_t flat_int:1;
    };

    struct R8_IntStatus1:public Reg8<>{
        static constexpr RegAddress address = 0x1d; // INT_STATUS_3
        uint8_t __resv__:2;
        uint8_t highg_int:1;
        uint8_t lowg_int:1;
        uint8_t drdy_int:1;
        uint8_t ffull_int:1;
        uint8_t sfwm_int:1;
        uint8_t nomo_int:1;
    };

    struct R8_IntStatus2:public Reg8<>{
        static constexpr RegAddress address = 0x1d; // INT_STATUS_3
        
        uint8_t anym_first_x: 1;
        uint8_t anym_first_y: 1;
        uint8_t anym_first_z: 1;
        uint8_t anym_sign: 1;

        uint8_t tap_first_z: 1;
        uint8_t tap_first_x: 1;
        uint8_t tap_first_y: 1;
        uint8_t tap_sign: 1;
    };


    struct R8_IntStatus3:public Reg8<>{
        static constexpr RegAddress address = 0x1e; // INT_STATUS_3
        uint8_t high_first_x: 1;
        uint8_t high_first_y: 1;
        uint8_t high_first_z: 1;
        uint8_t high_sign: 1;

        uint8_t orient_2: 1;
        uint8_t orient_1_0: 2;
        uint8_t flat: 1;
    };

    struct R8_Temperature:public Reg8<>{
        static constexpr RegAddress address = 0x21; // TEMPERATURE_1
        uint16_t temperature: 16; // Assuming 8 bits for temperature
    };

    struct R8_FifoLength:public Reg8<>{
        static constexpr RegAddress address = 0x23; // FIFO_LENGTH_1
        uint16_t fifo_byte_counter: 11; // Assuming 8 bits for temperature
        uint16_t __resv__:5;
    };
    
    struct R8_FifoData:public Reg8<>{
        static constexpr RegAddress address = 0x25;
        uint8_t data;
    };

    struct R8_AccConf:public Reg8<>{
        static constexpr RegAddress address = 0x40;
        AccOdr acc_odr:4;
        uint8_t acc_bwp:3;

        //undersample enable
        uint8_t acc_us:1;
    }DEF_R8(acc_conf)
    
    struct R8_AccFs:public Reg8<>{
        static constexpr RegAddress address = 0x41;
        AccFs acc_fs:4;
        uint8_t __resv__:4;
    }DEF_R8(acc_fs)

    struct R8_GyrConf:public Reg8<>{
        static constexpr RegAddress address = 0x42;
        GyrOdr gyr_odr:4;
        uint8_t gyr_bwp:2;
        uint8_t __resv__:2 = 0;
    }DEF_R8(gyr_conf)

    struct R8_GyrFs:public Reg8<>{
        static constexpr RegAddress address = 0x43;
        GyrFs gyr_fs:3;
        uint8_t __resv__:5 = 0;
    }DEF_R8(gyr_fs)

    struct R8_MagConf:public Reg8<>{
        static constexpr RegAddress address = 0x44;
        uint8_t mag_odr:4;
        uint8_t __resv__:4;
    };

    struct R8_FifoDowns:public Reg8<>{
        static constexpr uint8_t address = 0x45;
        uint8_t gyr_fifo_down : 3; // 8 bits for acc_fifo_filt_data
        uint8_t gyr_fifo_filt_data:1;
        uint8_t acc_fifo_data : 3; // 8 bits for gyr_fifo_filt_data
        uint8_t acc_fifo_filt_data:1;
    };

    struct R8_FifoConfig0:public Reg8<>{
        static constexpr uint8_t address = 0x46;
        uint8_t fifo_water_mark : 8; // 8 bits for fifo_water_mark
    };

    struct R8_FifoConfig1:public Reg8<>{
        static constexpr uint8_t address = 0x47;

        uint8_t __resv__ : 1;   // 1 bit reserved
        uint8_t fifo_time_en : 1; // 1 bit for fifo_time_en
        uint8_t fifo_tag_int2_en : 1; // 1 bit for fifo_tag_int2_en
        uint8_t fifo_tag_int1_en : 1; // 1 bit for fifo_tag_int1_en

        uint8_t fifo_header_en : 1; // 1 bit for fifo_header_en
        uint8_t fifo_mag_en : 1; // 1 bit for fifo_mag_en
        uint8_t fifo_acc_en : 1; // 1 bit for fifo_acc_en
        uint8_t fifo_gyr_en : 1; // 1 bit for fifo_gyr_en
    };

    struct R8_MagIf0:public Reg8<>{
        static constexpr uint8_t address = 0x4B;
        uint8_t __resv__:1;
        uint8_t i2c_device_addr: 7; // Reserved (no specific fields)
    };

    struct R8_MagIf1:public Reg8<>{
        static constexpr uint8_t address = 0x4C;
        uint8_t mag_rd_burst:2;
        uint8_t mag_offset:4;
        uint8_t __resv__ : 1;   // 7 bits reserved
        uint8_t mag_manual_en : 1; // 1 bit for mag_manual_en
    };

    struct R8_MagIf2:public Reg8<>{
        static constexpr uint8_t address = 0x4D;
        uint8_t read_addr : 8; // 8 bits for read_addr
    };

    struct R8_MagIf3:public Reg8<>{
        static constexpr uint8_t address = 0x4E;
        uint8_t write_addr : 8; // 8 bits for write_addr
    };

    struct R8_MagIf4:public Reg8<>{
        static constexpr uint8_t address = 0x4F;
        uint8_t write_data : 8; // 8 bits for write_data
    };

    struct R8_IntEn0:public Reg8<>{
        static constexpr uint8_t address = 0x50;
        
        uint8_t int_anymo_x_en:1;
        uint8_t int_anymo_y_en:1;
        uint8_t int_anymo_z_en:1;
        uint8_t :1;

        uint8_t int_d_tap_en : 1;
        uint8_t int_s_tap_en : 1;
        uint8_t int_orient_en : 1;
        uint8_t int_flat_en : 1;
    };

    struct R8_IntEn1:public Reg8<>{
        static constexpr uint8_t address = 0x51;
        uint8_t int_highg_x_en : 1;
        uint8_t int_highg_y_en : 1;
        uint8_t int_highg_z_en : 1;
        uint8_t int1_low_en : 1;

        uint8_t int1_drdy_en : 1;
        uint8_t int1_full_en : 1;
        uint8_t int1_fwm_en : 1;
        uint8_t : 1; // Reserved
    };

    struct R8_IntEn2:public Reg8<>{
        static constexpr uint8_t address = 0x52;
        uint8_t int_nomox_en:1;
        uint8_t int_nomoy_en:1;
        uint8_t int_nomoz_en:1;
        uint8_t int_step_det_en:1;
        uint8_t : 4; // Reserved
    };

    struct R8_IntOutCtrl:public Reg8<>{
        static constexpr uint8_t address = 0x53;
        uint8_t int1_edge_ctrl:1;
        uint8_t int1_lvl:1;
        uint8_t int1_od:1;
        uint8_t int1_output_en;
        uint8_t int2_edge_ctrl:1;
        uint8_t int2_lvl:1;
        uint8_t int2_od:1;
        uint8_t int2_output_en:1;
    };

    struct R8_IntLatch:public Reg8<>{
        static constexpr uint8_t address = 0x54;
        uint8_t int_latch:4;
        uint8_t int1_input_en:1;
        uint8_t int2_input_en:1;
        uint8_t : 2; // Reserved
    };

    struct R8_IntMap0:public Reg8<>{
        static constexpr uint8_t address = 0x55;

        uint8_t int1_lowg_step:1;
        uint8_t int1_highg:1;
        uint8_t int1_anymotion:1;
        uint8_t int1_nomotiom:1;

        uint8_t int1_d_tap : 1;
        uint8_t int1_s_tap : 1;
        uint8_t int1_orient : 1;
        uint8_t int1_flat : 1;
    };

    struct R8_IntMap1:public Reg8<>{
        static constexpr uint8_t address = 0x56;
        uint8_t int2_pmu_trig:1;
        uint8_t int2_fful:1;
        uint8_t int2_fwm:1;
        uint8_t int2_drdy : 1;

        uint8_t int1_pmu_trig:1;
        uint8_t int1_fful:1;
        uint8_t int1_fwm:1;
        uint8_t int1_drdy : 1;
    };

    struct R8_IntMap2:public Reg8<>{
        static constexpr uint8_t address = 0x57;

        uint8_t int2_lowg_step:1;
        uint8_t int2_highg:1;
        uint8_t int2_anymotion:1;
        uint8_t int2_nomotiom:1;

        uint8_t int2_d_tap : 1;
        uint8_t int2_s_tap : 1;
        uint8_t int2_orient : 1;
        uint8_t int2_flat : 1;
    };

    struct R8_IntData0:public Reg8<>{
        static constexpr uint8_t address = 0x58;
        uint8_t:3;
        uint8_t int_tap_src:1;
        uint8_t:3;
        uint8_t int_low_high_src:1;
    };

    struct R8_IntData1:public Reg8<>{
        static constexpr uint8_t address = 0x59;
        uint8_t:7;
        uint8_t int_motion_src:1;
    };

    struct R8_IntLowHigh0:public Reg8<>{
        static constexpr uint8_t address = 0x5A;
        uint8_t int_low_dur;
    };

    struct R8_IntLowHigh1:public Reg8<>{
        static constexpr uint8_t address = 0x5B;
        uint8_t int_low_th;
    };

    struct R8_IntLowHigh2:public Reg8<>{
        static constexpr uint8_t address = 0x5C;
        uint8_t int_low_hy:2;
        uint8_t :4;
        uint8_t int_high_hy:2;
    };

    struct R8_IntLowHigh3:public Reg8<>{
        static constexpr uint8_t address = 0x5D;
        uint8_t int_high_dur;
    };

    struct R8_IntLowHigh4:public Reg8<>{
        static constexpr uint8_t address = 0x5E;
        uint8_t int_high_th;
    };

    struct R8_IntMotion0:public Reg8<>{
        static constexpr uint8_t address = 0x5F;
        uint8_t int_anym_dur:2;
        uint8_t int_slo_nomo_dur:6;
    };

    struct R8_IntMotion1:public Reg8<>{
        static constexpr uint8_t address = 0x60;
        uint8_t int_anymo_th;
    };

    struct R8_IntMotion2:public Reg8<>{
        static constexpr uint8_t address = 0x61;
        uint8_t int_slo_nomo_th;
    };

    struct R8_SelfTest:public Reg8<>{
        static constexpr uint8_t address = 0x6D;
        uint8_t acc_self_test_en:2;
        uint8_t acc_self_test_sign:1;
        uint8_t acc_self_test_amp:1;
        uint8_t gyr_self_test_en:1; 
        uint8_t :3;
    }DEF_R8(self_test)


    Vector3<int16_t> gyr = {};
    Vector3<int16_t> acc = {};

};

}