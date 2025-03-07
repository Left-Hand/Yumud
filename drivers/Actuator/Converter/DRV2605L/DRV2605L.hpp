#pragma once

#include "drivers/device_defs.h"

// #define DRV2605L_DEBUG

namespace ymd::drivers{

class DRV2605L{
public:
    enum class Package:uint8_t{
        _2605,
        _2604,
        _2604L,
        _2605L
    };

    scexpr uint8_t default_i2c_addr = 0b01100000;

    DRV2605L(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    DRV2605L(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    DRV2605L(hal::I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(hal::I2cDrv(i2c, addr)){;}



protected:
    using RegAddress = uint8_t;

    hal::I2cDrv i2c_drv_;


    enum class Mode:uint8_t{
        // 0: Internal trigger
        // Waveforms are fired by setting the GO bit in register 0x0C.

        // 1: External trigger (edge mode)
        // A rising edge on the IN/TRIG pin sets the GO Bit. A second rising
        // edge on the IN/TRIG pin cancels the waveform if the second rising
        // edge occurs before the GO bit has cleared.

        // 2: External trigger (level mode)
        // The GO bit follows the state of the external trigger. A rising edge on
        // the IN/TRIG pin sets the GO bit, and a falling edge sends a cancel. If
        // the GO bit is already in the appropriate state, no change occurs.
        
        // 3: PWM input and analog input
        // A PWM or analog signal is accepted at the IN/TRIG pin and used as
        // the driving source. The device actively drives the actuator while in
        // this mode. The PWM or analog input selection occurs by using the
        // N_PWM_ANALOG bit.

        // 4: Audio-to-vibe
        // An AC-coupled audio signal is accepted at the IN/TRIG pin. The
        // device converts the audio signal into meaningful haptic vibration. The
        // AC_COUPLE and N_PWM_ANALOG bits should also be set.

        // 5: Real-time playback (RTP mode)
        // The device actively drives the actuator with the contents of the
        // RTP_INPUT[7:0] bit in register 0x02.

        // 6: Diagnostics
        // Set the device in this mode to perform a diagnostic test on the
        // actuator. The user must set the GO bit to start the test. The test is
        // complete when the GO bit self-clears. Results are stored in the
        // DIAG_RESULT bit in register 0x00.

        // 7: Auto calibration
        // Set the device in this mode to auto calibrate the device for the
        // actuator. Before starting the calibration, the user must set the all
        // required input parameters. The user must set the GO bit to start the
        // calibration. Calibration is complete when the GO bit self-clears. For
        // more information see the Auto Calibration Procedure section.

        InternalTrigger,
        ExternalTriggerEdgeMode,
        ExternalTriggerLevelMode,
        PwmInputAndAnalogInput,
        AudioToVibe,
        RealTimePlayback,
        Diagnostics,
        AutoCalibration
    };

    struct StatusReg:public RegC8<>{
        scexpr RegAddress address = 0x00;

        uint8_t oc_detect:1;
        uint8_t over_temp:1;
        uint8_t :1;
        uint8_t diag_result:1;
        uint8_t :1;
        uint8_t device_id;
    };

    struct ModeReg:public Reg8<>{
        scexpr RegAddress address = 0x01;

        uint8_t mode:3;
        uint8_t :3;
        uint8_t standby:1;
        uint8_t dev_reset:1;

        using Reg8::operator=;
    };

    struct RtpReg:public Reg8<>{
        scexpr RegAddress address = 0x02;

        uint8_t rtp_input:8;
    };

    struct LibrarySelReg:public Reg8<>{
        scexpr RegAddress address = 0x03;

        uint8_t lib_sel:3;
        uint8_t :1;
        uint8_t hiz:1;
        uint8_t :3;
    };

    struct WaveformSegReg:public Reg8<>{
        scexpr RegAddress address = 0x04;
        scexpr RegAddress address_end = 0x0B;

        uint8_t wav_frm_seq:7;
        uint8_t wait:1;
    };

    struct GoReg:public Reg8<>{
        scexpr RegAddress address = 0x0C;

        uint8_t go:1;
        uint8_t :7;
    };

    struct OverrideTimeOffset:public Reg8<>{
        scexpr RegAddress address = 0x0D;

        uint8_t odt:8;
    };

    struct SustainTimeOffsetPositiveReg:public Reg8<>{
        scexpr RegAddress address = 0x0E;

        uint8_t :8;
    };

    struct SustainTimeOffsetNegitiveReg:public Reg8<>{
        scexpr RegAddress address = 0x0F;

        uint8_t :8;
    };

    struct BrakeTimeOffsetReg:public Reg8<>{
        scexpr RegAddress address = 0x10;

        uint8_t :8;
    };


    enum class A2V_LPF_Freq{
        _100Hz,
        _125Hz,
        _150Hz,
        _200Hz
    };

    enum class A2V_Peak_Time{
        _10ms,
        _20ms,
        _30ms,
        _40ms,

    };
    struct Audio2VibeReg:public Reg8<>{
        scexpr RegAddress address = 0x11;

        uint8_t ath_filter:2;
        uint8_t ath_peak_time:2;
        uint8_t :4;
    };

    
    struct Audio2VibeMinimalInputLevelReg:public Reg8<>{
        scexpr RegAddress address = 0x12;

        uint8_t :8;
    };

    
    struct Audio2VibeMaxmalInputLevelReg:public Reg8<>{
        scexpr RegAddress address = 0x13;

        uint8_t :8;
    };

    struct Audio2VibeMinimalOutputDriveReg:public Reg8<>{
        scexpr RegAddress address = 0x14;

        uint8_t :8;
    };


    struct Audio2VibeMaxmalOutputDriveReg:public Reg8<>{
        scexpr RegAddress address = 0x15;

        uint8_t :8;
    };

    struct RatedVoltageReg:public Reg8<>{
        scexpr RegAddress address = 0x16;

        uint8_t :8;
    };

    struct OverdriveClampVoltageReg:public Reg8<>{
        scexpr RegAddress address = 0x17;

        uint8_t :8;
    };

    struct AutoCaliCompResultReg:public Reg8<>{
        scexpr RegAddress address = 0x18;

        uint8_t :8;
    };

    struct AutoCaliBackEMFReg:public Reg8<>{
        scexpr RegAddress address = 0x19;

        uint8_t :8;
    };

    enum class FbBrakeFactor:uint8_t{
        _1x,
        _2x,
        _3x,
        _4x,
        _6x,
        _8x,
        _16x,
    };

    enum class LoopGain:uint8_t{
        Low = 0,
        Medium,
        High,
        VeryHigh
    };

    enum class BemfGain:uint8_t{
        _0_33x,
        _1x,
        _1_8x,
        _4x,
        _5x,
        _10x,
        _20x,
        _30x
    };

    struct FeedbackControlReg:public Reg8<>{
        scexpr RegAddress address = 0x1a;

        uint8_t bemf_gain:2;
        uint8_t loop_gain:2;
        uint8_t fb_brake_factor:3;

        // 0: ERM Mode
        // 1: LRA Mode
        uint8_t n_erm_lra:1;
    };

    struct Control1Reg:public Reg8<>{
        scexpr RegAddress address = 0x1b;

        uint8_t drive_time:5;
        uint8_t ac_couple:1;
        uint8_t :1;
        uint8_t startup_boost:1;
    };

    struct Control2Reg:public Reg8<>{
        scexpr RegAddress address = 0x1c;

        uint8_t idiss_time:2;
        uint8_t blanking_time:2;
        uint8_t sample_time:2;
        uint8_t brake_stabilizer:2;
        uint8_t bidir_input:2;
    };

    struct Control3Reg:public Reg8<>{
        scexpr RegAddress address = 0x1d;

        uint8_t lra_openloop:1;
        uint8_t n_pwm_analog:1;
        uint8_t lra_drive_mode:1;
        uint8_t data_format_rtp:1;
        uint8_t supply_comp_dis:1;
        uint8_t erm_openloop:1;
        uint8_t ng_thresh:1;
    };

    struct Control4Reg:public Reg8<>{
        scexpr RegAddress address = 0x1e;

        uint8_t otp_program:1;
        uint8_t :1;
        uint8_t otp_status:1;
        uint8_t :1;
        uint8_t autocal_time:2;
        uint8_t :2;
    };

    struct VbatVoltageMonitorReg:public Reg8<>{
        scexpr RegAddress address = 0x1e;

        uint8_t :8;
    };

    struct LRA_ResonancePeriodReg:public Reg8<>{
        scexpr RegAddress address = 0x1f;

        uint8_t :8;
    };
    
    StatusReg status_reg = {};
    ModeReg mode_reg = {};
    RtpReg rtp_reg = {};
    LibrarySelReg library_sel_reg = {};
    WaveformSegReg waveform_seg_reg = {};
    GoReg go_reg = {};
    OverrideTimeOffset override_time_offset_reg = {};
    SustainTimeOffsetPositiveReg sustain_time_offset_positive_reg = {};
    SustainTimeOffsetNegitiveReg sustain_time_offset_negitive_reg = {};
    BrakeTimeOffsetReg brake_time_offset_reg = {};
    Audio2VibeReg audio2vibe_reg = {};
    Audio2VibeMinimalInputLevelReg audio2vibe_minimal_input_level_reg = {};
    Audio2VibeMaxmalInputLevelReg audio2vibe_maxmal_input_level_reg = {};
    Audio2VibeMinimalOutputDriveReg audio2vibe_minimal_output_drive_reg = {};
    Audio2VibeMaxmalOutputDriveReg audio2vibe_maxmal_output_drive_reg = {};
    RatedVoltageReg rated_voltage_reg = {};
    OverdriveClampVoltageReg overdrive_clamp_voltage_reg = {};
    AutoCaliCompResultReg auto_cali_comp_result_reg = {};
    AutoCaliBackEMFReg auto_cali_back_emf_reg = {};
    FeedbackControlReg feedback_control_reg = {};
    Control1Reg control1_reg = {};
    Control2Reg control2_reg = {};

    auto writeReg(const RegAddress address, const uint8_t reg){
        return i2c_drv_.writeReg<uint8_t>((uint8_t)address, reg);
    }

    auto readReg(const RegAddress address, uint8_t & reg){
        return i2c_drv_.readReg<uint8_t>((uint8_t)address, reg);
    }

    auto requestBurst(const RegAddress addr, uint8_t * data, size_t len){
        return i2c_drv_.readMulti((uint8_t)addr, data, len);
    }


    void setFbBrakeFactor(const FbBrakeFactor factor);
    void setFbBrakeFactor(const int fractor);
public:
    void reset();
    void update();
    bool verify();
    bool init();
    Package getPackage();
    void setBemfGain(const BemfGain gain);
    void setLoopGain(const LoopGain gain);
    void play(const uint8_t idx);
    bool autocal();
};

}