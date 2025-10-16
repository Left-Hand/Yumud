#pragma once

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"

namespace ymd::drivers{

struct BMI270_Prelude{
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0x68 >> 1);

    using RegAddr = uint8_t;


    enum class DPS:uint8_t{
        _250, _500, _1000, _2000
    };

    enum class G:uint8_t{
        _2, _4, _8, _16
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
        SUSPEND,
        NORMAL,
        LOW_POWER,
        FAST_SETUP
    };

};


struct BMI270_Regset:public BMI270_Prelude{

    struct R8_ChipId:public Reg8<>{
        static constexpr RegAddr address = 0x00;

        static constexpr uint8_t KEY = 0x24;
        uint8_t data;
    };

    struct R8_Err:public Reg8<>{
        static constexpr RegAddr address = 0x02;

        uint8_t fatal_err:1;
        uint8_t internal_err:4;
        uint8_t :1;
        uint8_t fifo_err:1;
        uint8_t aux_err:1;
    };

    struct R8_Status:public Reg8<>{
        static constexpr RegAddr address = 0x03;

        uint8_t :2;
        uint8_t aux_busy:1;
        uint8_t :1;
        uint8_t cmd_rdy:1;
        uint8_t drdy_aux:1;
        uint8_t drdy_gyr:1;
        uint8_t drdy_acc:1;
    };

    struct R8_Data0:public Reg8<>{
        static constexpr RegAddr address = 0x04;
        static constexpr size_t size = 20;

        uint8_t :8;
    };

    struct R8_SensorTime0:public Reg8<>{
        static constexpr RegAddr address = 0x18;
        static constexpr size_t size = 3;

        uint8_t :8;
    };

    struct R8_Event:public Reg8<>{
        static constexpr RegAddr address = 0x1B;
        uint8_t por_detected:1;
        uint8_t :1;
        uint8_t error_code:3;
        uint8_t :3;
    };

    struct R8_IntStatus0:public Reg8<>{
        static constexpr RegAddr address = 0x1C;

        uint8_t sign_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct R8_IntStatus1:public Reg8<>{
        static constexpr RegAddr address = 0x1D;

        uint8_t full_int:1;
        uint8_t fwm_int:1;
        uint8_t err_int:1;
        uint8_t :2;
        uint8_t aux_drdy_int:1;
        uint8_t gyr_drdy_int:1;
        uint8_t acc_drdy_int:1;
    };

    struct R8_ScOut:public Reg16<>{
        static constexpr RegAddr address = 0x1E;

        uint16_t :16;
    };

    struct R8_WrGestOut:public Reg8<>{
        static constexpr RegAddr address = 0x20;

        uint8_t wr_gest_out:3;
        uint8_t act_out:2;
        uint8_t :3;
    };

    struct R8_InternalStatus:public Reg8<>{
        static constexpr RegAddr address = 0x21;

        uint8_t message:4;
        uint8_t :1;
        uint8_t axes_remap_error:1;
        uint8_t odr_50hz_err:1;
        uint8_t :1;
    };

    struct R16_Temperature:public Reg16<>{
        static constexpr RegAddr address = 0x22;

        uint16_t :16;
    };

    struct R16_FifoLength:public Reg16<>{
        static constexpr RegAddr address = 0x24;

        uint16_t data:14;
        uint16_t :2;
    };

    struct R8_FifoData:public Reg8<>{
        static constexpr RegAddr address = 0x26;

        uint8_t :8;
    };

    struct R8_FeatPage:public Reg8<>{
        static constexpr RegAddr address = 0x2F;

        uint8_t page:3;
        uint8_t :5;
    };


    //...

    struct R8_AccConf:public Reg8<>{
        static constexpr RegAddr address = 0x40;

        uint8_t acc_odr:4;
        uint8_t acc_bwp:3;
        uint8_t acc_filter_perf:1;
    };

    struct R8_AccFs:public Reg8<>{
        static constexpr RegAddr address = 0x41;

        uint8_t acc_range:2;
        uint8_t :6;
    };

    struct R8_GyrConf:public Reg8<>{
        static constexpr RegAddr address = 0x42;

        uint8_t gyr_odr:4;
        uint8_t gyr_bwp:2;
        uint8_t gyr_noise_perf:1;
        uint8_t gyr_filter_perf:1;
    };

    struct R8_GyrFs:public Reg8<>{
        static constexpr RegAddr address = 0x43;

        uint8_t gyr_range:3;
        uint8_t ois_range:1;
        uint8_t :4;
    };

    struct R8_AuxConf:public Reg8<>{
        static constexpr RegAddr address = 0x44;

        uint8_t aux_odr:4;
        uint8_t aux_offset:4;
    };


    struct R8_FifoDowns:public Reg8<>{
        static constexpr RegAddr address = 0x45;
        uint8_t gyr_fifo_downs:3;
        uint8_t gyr_fifo_filt_data:1;
        uint8_t acc_fifo_downs:1;
        uint8_t acc_fifo_filt_data:1;
    };

    struct R8_FifoWtm:public Reg16<>{
        static constexpr RegAddr address = 0x46;
        uint8_t fifo_stop_on_full:1;
        uint8_t fifo_time_en:1;
        uint8_t :6;
    };

    struct R8_FifoConfig0:public Reg8<>{
        static constexpr RegAddr address = 0x48;
        uint8_t fifo_overrun:1;
    };


    struct R8_FifoConfig1:public Reg8<>{
        static constexpr RegAddr address = 0x49;

        uint8_t fifo_tag_int1_en:2;
        uint8_t fifo_tag_int2_en:2;
        uint8_t fifo_header_en:1;
        uint8_t fifo_aux_en:1;
        uint8_t fifo_acc_en:1;
        uint8_t fifo_gyr_en:1;
    };

    struct R8_Saturation:public Reg8<>{
        static constexpr RegAddr address = 0x4A;

        uint8_t acc_x:1;
        uint8_t acc_y:1;
        uint8_t acc_z:1;

        uint8_t gyr_x:1;
        uint8_t gyr_y:1;
        uint8_t gyr_z:1;

        uint8_t :2;
    };

    struct R8_AuxDevId:public Reg8<>{
        static constexpr RegAddr address = 0x4b;

        uint8_t :1;
        uint8_t i2c_device_addr;
    };

    struct R8_AuxIfConf:public Reg8<>{
        static constexpr RegAddr address = 0x4c;

        uint8_t aux_rd_burst:2;
        uint8_t man_rd_burst:2;
        uint8_t :2;
        uint8_t aux_fcu_write_en:1;
        uint8_t aux_manual_en:1;
    };

    struct R8_AuxRdAddr:public Reg8<>{
        static constexpr RegAddr address = 0x4d;

        uint8_t :8;
    };

    struct R8_AuxWrAddr:public Reg8<>{
        static constexpr RegAddr address = 0x4e;

        uint8_t :8;
    };

    struct R8_AuxWrData:public Reg8<>{
        static constexpr RegAddr address = 0x4d;

        uint8_t :8;
    };

    struct R8_ErrRegMsk:public Reg8<>{
        static constexpr RegAddr address = 0x52;

        uint8_t fatal_err:1;
        uint8_t internal_err:4;
        uint8_t :1;
        uint8_t fifo_err:1;
        uint8_t aux_err:1;
    };

    struct R8_IntIoCtrl1:public Reg8<>{
        static constexpr RegAddr address = 0x53;

        uint8_t :1;
        uint8_t lvl:1;
        uint8_t od:1;
        uint8_t output_en:1;
        uint8_t input_en:1;
        uint8_t :3;
    };

    struct R8_IntIoCtrl2:public Reg8<>{
        static constexpr RegAddr address = 0x54;

        uint8_t :1;
        uint8_t lvl:1;
        uint8_t od:1;
        uint8_t output_en:1;
        uint8_t input_en:1;
        uint8_t :3;
    };

    struct R8_IntLatch:public Reg8<>{
        static constexpr RegAddr address = 0x55;

        uint8_t int_latch:1;
        uint8_t:7;
    };

    struct R8_Int1MapFeat:public Reg8<>{
        static constexpr RegAddr address = 0x56;

        uint8_t sig_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesuture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct R8_Int2MapFeat:public Reg8<>{
        static constexpr RegAddr address = 0x57;

        uint8_t sig_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesuture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct R8_IntCtrl:public Reg8<>{
        static constexpr RegAddr address = 0x58;

        uint8_t ffull_int1:1;
        uint8_t fwm_int1:1;
        uint8_t drdy_int1:1;
        uint8_t err_int1:1;

        uint8_t ffull_int2:1;
        uint8_t fwm_int2:1;
        uint8_t drdy_int2:1;
        uint8_t err_int2:1;
    };

    struct R8_InitCtrl:public Reg8<>{
        static constexpr RegAddr address = 0x59;
        uint8_t :8;
    };

    //0x5a resv

    struct R16_InitAddr:public Reg16<>{
        static constexpr RegAddr address = 0x5B;

        uint16_t base_0_3:4;
        uint16_t :4;
        uint16_t base_11_4:8;
    };

    //0x5d resv

    struct R8_InitData:public Reg16<>{
        static constexpr RegAddr address = 0x5E;

        uint8_t :8;
    };

    struct R8_InternalError:public Reg8<>{
        uint8_t :1;
        uint8_t int_err_1:1;
        uint8_t int_err_2:1;
        uint8_t :1;
        uint8_t feat_eng_disabled:1;
        uint8_t :3;
    };

    struct R8_AuxIfTrm:public Reg8<>{
        static constexpr RegAddr address = 0x68;

        uint8_t asda_pupsel:2;
        uint8_t spare3:1;
    };

    struct R8_GyrCrtConf:public Reg8<>{
        static constexpr RegAddr address = 0x69;

        uint8_t:2;
        uint8_t crt_running1:1;
        uint8_t rdy_for_dl:1;
        uint8_t :4;
    };

    struct R8_NvmConf:public Reg8<>{
        static constexpr RegAddr address = 0x6A;

        uint8_t:1;
        uint8_t nvm_prog_en:1;
        uint8_t:6;
    };

    struct R8_IfConf:public Reg8<>{
        static constexpr RegAddr address = 0x6B;

        uint8_t spi3:1;
        uint8_t spi3_ois:1;
        uint8_t :2;
        uint8_t ois_en:1;
        uint8_t aux_en:1;
        uint8_t :2;
    };

    struct R8_Dev:public Reg8<>{
        static constexpr RegAddr address = 0x6c;

        uint8_t io_pad_drv1:3;
        uint8_t io_pad_i2c_b1:1;
        uint8_t io_pad_drv2:3;
        uint8_t io_pad_i2c_b2:1;
    };

    struct R8_AccSelfTest:public Reg8<>{
        static constexpr RegAddr address = 0x6D;

        uint8_t acc_self_test_en:1;
        uint8_t:1;
        uint8_t acc_self_test_sign:1;
        uint8_t acc_self_test_amp:1;
        uint8_t:4;
    };


    struct R8_GyeoSelfTest:public Reg8<>{
        static constexpr RegAddr address = 0x6E;

        uint8_t gyr_st_axes_done:1;
        uint8_t gyr_axis_x_ok:1;
        uint8_t gyr_axis_y_ok:1;
        uint8_t gyr_axis_z_ok:1;
        uint8_t:4;
    };

    struct R8_NvConf:public Reg8<>{
        static constexpr RegAddr address = 0x70;

        uint8_t spi_en:1;
        uint8_t i2c_wdt_sel:1;
        uint8_t i2c_wdy_en:1;
        uint8_t acc_off_en:1;
        uint8_t:4;
    };


    struct R8_PwrConf{
        static constexpr RegAddr address = 0x7C;

        uint8_t adv_power_save:1;
        uint8_t fifo_self_wake_up:1;
        uint8_t fup_en:1;
        uint8_t :5;
    };

    struct R8_PweCtrl{
        static constexpr RegAddr address = 0x7D;
        uint8_t aux_en:1;
        uint8_t gyr_en:1;
        uint8_t acc_en:1;
        uint8_t temp_en:1;
        uint8_t :4;
    };


    // ChipIdReg chipid_reg = {};
    // uint8_t :8;
    // ErrReg err_reg = {};
    // StatusReg status_reg = {};

    // uint8_t data_regs[Data0Reg::size] = {};
    // uint8_t sensor_time_regs[SensorTime0Reg::size] = {};

    // EventReg event_reg = {};
    
    // IntStatus0Reg int_status0_reg = {};
    // IntStatus1Reg int_status1_reg = {};
};

}