
    struct ChipIdReg:public Reg8<>{
        scexpr RegAddress address = 0x00;

        scexpr uint8_t correct = 0x24;
        uint8_t data;
    };

    struct ErrReg:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t fatal_err:1;
        uint8_t internal_err:4;
        uint8_t :1;
        uint8_t fifo_err:1;
        uint8_t aux_err:1;
    };

    struct StatusReg:public Reg8<>{
        scexpr RegAddress address = 0x03;

        uint8_t :2;
        uint8_t aux_busy:1;
        uint8_t :1;
        uint8_t cmd_rdy:1;
        uint8_t drdy_aux:1;
        uint8_t drdy_gyr:1;
        uint8_t drdy_acc:1;
    };

    struct Data0Reg:public Reg8<>{
        scexpr RegAddress address = 0x04;
        scexpr size_t size = 20;

        uint8_t :8;
    };

    struct SensorTime0Reg:public Reg8<>{
        scexpr RegAddress address = 0x18;
        scexpr size_t size = 3;

        uint8_t :8;
    };

    struct EventReg:public Reg8<>{
        scexpr RegAddress address = 0x1B;
        uint8_t por_detected:1;
        uint8_t :1;
        uint8_t error_code:3;
        uint8_t :3;
    };

    struct IntStatus0Reg:public Reg8<>{
        scexpr RegAddress address = 0x1C;

        uint8_t sign_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct IntStatus1Reg:public Reg8<>{
        scexpr RegAddress address = 0x1D;

        uint8_t full_int:1;
        uint8_t fwm_int:1;
        uint8_t err_int:1;
        uint8_t :2;
        uint8_t aux_drdy_int:1;
        uint8_t gyr_drdy_int:1;
        uint8_t acc_drdy_int:1;
    };

    struct ScOutReg:public Reg16<>{
        scexpr RegAddress address = 0x1E;

        uint16_t :16;
    };

    struct WrGestOutReg:public Reg8<>{
        scexpr RegAddress address = 0x20;

        uint8_t wr_gest_out:3;
        uint8_t act_out:2;
        uint8_t :3;
    };

    struct InternalStatusReg:public Reg8<>{
        scexpr RegAddress address = 0x21;

        uint8_t message:4;
        uint8_t :1;
        uint8_t axes_remap_error:1;
        uint8_t odr_50hz_err:1;
        uint8_t :1;
    };

    struct TemperatureReg:public Reg16<>{
        scexpr RegAddress address = 0x22;

        uint16_t :16;
    };

    struct FifoLengthReg:public Reg16<>{
        scexpr RegAddress address = 0x24;

        uint16_t data:14;
        uint16_t :2;
    };

    struct FifoDataReg:public Reg8<>{
        scexpr RegAddress address = 0x26;

        uint8_t :8;
    };

    struct FeatPageReg:public Reg8<>{
        scexpr RegAddress address = 0x2F;

        uint8_t page:3;
        uint8_t :5;
    };


    //...

    struct AccConfReg:public Reg8<>{
        scexpr RegAddress address = 0x40;

        uint8_t acc_odr:4;
        uint8_t acc_bwp:3;
        uint8_t acc_filter_perf:1;
    };

    struct AccRangeReg:public Reg8<>{
        scexpr RegAddress address = 0x41;

        uint8_t acc_range:2;
        uint8_t :6;
    };

    struct GyrConfReg:public Reg8<>{
        scexpr RegAddress address = 0x42;

        uint8_t gyr_odr:4;
        uint8_t gyr_bwp:2;
        uint8_t gyr_noise_perf:1;
        uint8_t gyr_filter_perf:1;
    };

    struct GyrRangeReg:public Reg8<>{
        scexpr RegAddress address = 0x43;

        uint8_t gyr_range:3;
        uint8_t ois_range:1;
        uint8_t :4;
    };

    struct AuxConfReg:public Reg8<>{
        scexpr RegAddress address = 0x44;

        uint8_t aux_odr:4;
        uint8_t aux_offset:4;
    };


    struct FifoDownsReg:public Reg8<>{
        scexpr RegAddress address = 0x45;
        uint8_t gyr_fifo_downs:3;
        uint8_t gyr_fifo_filt_data:1;
        uint8_t acc_fifo_downs:1;
        uint8_t acc_fifo_filt_data:1;
    };

    struct FifoWtmReg:public Reg16<>{
        scexpr RegAddress address = 0x46;
        uint8_t fifo_stop_on_full:1;
        uint8_t fifo_time_en:1;
        uint8_t :6;
    };

    struct FifoConfig0Reg:public Reg8<>{
        scexpr RegAddress address = 0x48;
        uint8_t fifo_overrun:1;
    };


    struct FifoConfig1Reg:public Reg8<>{
        scexpr RegAddress address = 0x49;

        uint8_t fifo_tag_int1_en:2;
        uint8_t fifo_tag_int2_en:2;
        uint8_t fifo_header_en:1;
        uint8_t fifo_aux_en:1;
        uint8_t fifo_acc_en:1;
        uint8_t fifo_gyr_en:1;
    };

    struct SaturationReg:public Reg8<>{
        scexpr RegAddress address = 0x4A;

        uint8_t acc_x:1;
        uint8_t acc_y:1;
        uint8_t acc_z:1;

        uint8_t gyr_x:1;
        uint8_t gyr_y:1;
        uint8_t gyr_z:1;

        uint8_t :2;
    };

    struct AuxDevIdReg:public Reg8<>{
        scexpr RegAddress address = 0x4b;

        uint8_t :1;
        uint8_t i2c_device_addr;
    };

    struct AuxIfConfReg:public Reg8<>{
        scexpr RegAddress address = 0x4c;

        uint8_t aux_rd_burst:2;
        uint8_t man_rd_burst:2;
        uint8_t :2;
        uint8_t aux_fcu_write_en:1;
        uint8_t aux_manual_en:1;
    };

    struct AuxRdAddrReg:public Reg8<>{
        scexpr RegAddress address = 0x4d;

        uint8_t :8;
    };

    struct AuxWrAddrReg:public Reg8<>{
        scexpr RegAddress address = 0x4e;

        uint8_t :8;
    };

    struct AuxWrDataReg:public Reg8<>{
        scexpr RegAddress address = 0x4d;

        uint8_t :8;
    };

    struct ErrRegMskReg:public Reg8<>{
        scexpr RegAddress address = 0x52;

        uint8_t fatal_err:1;
        uint8_t internal_err:4;
        uint8_t :1;
        uint8_t fifo_err:1;
        uint8_t aux_err:1;
    };

    struct IntIoCtrl1Reg:public Reg8<>{
        scexpr RegAddress address = 0x53;

        uint8_t :1;
        uint8_t lvl:1;
        uint8_t od:1;
        uint8_t output_en:1;
        uint8_t input_en:1;
        uint8_t :3;
    };

    struct IntIoCtrl2Reg:public Reg8<>{
        scexpr RegAddress address = 0x54;

        uint8_t :1;
        uint8_t lvl:1;
        uint8_t od:1;
        uint8_t output_en:1;
        uint8_t input_en:1;
        uint8_t :3;
    };

    struct IntLatchReg:public Reg8<>{
        scexpr RegAddress address = 0x55;

        uint8_t int_latch:1;
        uint8_t:7;
    };

    struct Int1MapFeatReg:public Reg8<>{
        scexpr RegAddress address = 0x56;

        uint8_t sig_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesuture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct Int2MapFeatReg:public Reg8<>{
        scexpr RegAddress address = 0x57;

        uint8_t sig_motion_out:1;
        uint8_t step_counter_out:1;
        uint8_t activity_out:1;
        uint8_t wrist_wear_wakeup_out:1;
        uint8_t wrist_gesuture_out:1;
        uint8_t no_motion_out:1;
        uint8_t any_motion_out:1;
        uint8_t :1;
    };

    struct IntCtrlReg:public Reg8<>{
        scexpr RegAddress address = 0x58;

        uint8_t ffull_int1:1;
        uint8_t fwm_int1:1;
        uint8_t drdy_int1:1;
        uint8_t err_int1:1;

        uint8_t ffull_int2:1;
        uint8_t fwm_int2:1;
        uint8_t drdy_int2:1;
        uint8_t err_int2:1;
    };

    struct InitCtrlReg:public Reg8<>{
        scexpr RegAddress address = 0x59;
        uint8_t :8;
    };

    //0x5a resv

    struct InitAddrReg:public Reg16<>{
        scexpr RegAddress address = 0x5B;

        uint16_t base_0_3:4;
        uint16_t :4;
        uint16_t base_11_4:8;
    };

    //0x5d resv

    struct InitDataReg:public Reg16<>{
        scexpr RegAddress address = 0x5E;

        uint8_t :8;
    };

    struct InternalErrorReg:public Reg8<>{
        uint8_t :1;
        uint8_t int_err_1:1;
        uint8_t int_err_2:1;
        uint8_t :1;
        uint8_t feat_eng_disabled:1;
        uint8_t :3;
    };

    struct AuxIfTrmReg:public Reg8<>{
        scexpr RegAddress address = 0x68;

        uint8_t asda_pupsel:2;
        uint8_t spare3:1;
    };

    struct GyrCrtConfReg:public Reg8<>{
        scexpr RegAddress address = 0x69;

        uint8_t:2;
        uint8_t crt_running1:1;
        uint8_t rdy_for_dl:1;
        uint8_t :4;
    };

    struct NvmConfReg:public Reg8<>{
        scexpr RegAddress address = 0x6A;

        uint8_t:1;
        uint8_t nvm_prog_en:1;
        uint8_t:6;
    };

    struct IfConfReg:public Reg8<>{
        scexpr RegAddress address = 0x6B;

        uint8_t spi3:1;
        uint8_t spi3_ois:1;
        uint8_t :2;
        uint8_t ois_en:1;
        uint8_t aux_en:1;
        uint8_t :2;
    };

    struct DevReg:public Reg8<>{
        scexpr RegAddress address = 0x6c;

        uint8_t io_pad_drv1:3;
        uint8_t io_pad_i2c_b1:1;
        uint8_t io_pad_drv2:3;
        uint8_t io_pad_i2c_b2:1;
    };

    struct AccSelfTestReg:public Reg8<>{
        scexpr RegAddress address = 0x6D;

        uint8_t acc_self_test_en:1;
        uint8_t:1;
        uint8_t acc_self_test_sign:1;
        uint8_t acc_self_test_amp:1;
        uint8_t:4;
    };


    struct GyeoSelfTestReg:public Reg8<>{
        scexpr RegAddress address = 0x6E;

        uint8_t gyr_st_axes_done:1;
        uint8_t gyr_axis_x_ok:1;
        uint8_t gyr_axis_y_ok:1;
        uint8_t gyr_axis_z_ok:1;
        uint8_t:4;
    };

    struct NvConfReg:public Reg8<>{
        scexpr RegAddress address = 0x70;

        uint8_t spi_en:1;
        uint8_t i2c_wdt_sel:1;
        uint8_t i2c_wdy_en:1;
        uint8_t acc_off_en:1;
        uint8_t:4;
    };


    struct PwrConfReg{
        scexpr RegAddress address = 0x7C;

        uint8_t adv_power_save:1;
        uint8_t fifo_self_wake_up:1;
        uint8_t fup_en:1;
        uint8_t :5;
    };

    struct PweCtrlReg{
        scexpr RegAddress address = 0x7D;
        uint8_t aux_en:1;
        uint8_t gyr_en:1;
        uint8_t acc_en:1;
        uint8_t temp_en:1;
        uint8_t :4;
    };
