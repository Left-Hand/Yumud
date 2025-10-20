#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"


namespace ymd::drivers{

struct DRV2605L_Prelude{
    using RegAddr = uint8_t;

    enum class Error_Kind:uint8_t{
        BusFault,
        Unspecified
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Package:uint8_t{
        _2605,
        _2604,
        _2604L,
        _2605L
    };

    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u7(0b0110000);

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
};

struct DRV2605L_Regs:public DRV2605L_Prelude{
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

    struct R8_Status:public RegC8<>{
        static constexpr RegAddr ADDRESS = 0x00;

        uint8_t oc_detect:1;
        uint8_t over_temp:1;
        uint8_t :1;
        uint8_t diag_result:1;
        uint8_t :1;
        uint8_t device_id;
    };

    struct R8_Mode:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x01;

        uint8_t mode:3;
        uint8_t :3;
        uint8_t standby:1;
        uint8_t dev_reset:1;

        using Reg8::operator=;
    };

    struct R8_Rtp:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x02;

        uint8_t rtp_input:8;
    };

    struct R8_LibrarySel:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x03;

        uint8_t lib_sel:3;
        uint8_t :1;
        uint8_t hiz:1;
        uint8_t :3;
    };

    struct R8_WaveformSeg:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x04;
        static constexpr RegAddr ADDRESS_end = 0x0B;

        uint8_t wav_frm_seq:7;
        uint8_t wait:1;
    };

    struct R8_Go:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0C;

        uint8_t go:1;
        uint8_t :7;
    };

    struct R8_OverrideTimeOffset:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0D;

        uint8_t odt:8;
    };

    struct R8_SustainTimeOffsetPositive:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0E;

        uint8_t :8;
    };

    struct R8_SustainTimeOffsetNegitive:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x0F;

        uint8_t :8;
    };

    struct R8_BrakeTimeOffset:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x10;

        uint8_t :8;
    };

    struct R8_Audio2Vibe:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x11;

        uint8_t ath_filter:2;
        uint8_t ath_peak_time:2;
        uint8_t :4;
    };

    
    struct R8_Audio2VibeMinimalInputLevel:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x12;

        uint8_t :8;
    };

    
    struct R8_Audio2VibeMaxmalInputLevel:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x13;

        uint8_t :8;
    };

    struct R8_Audio2VibeMinimalOutputDrive:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x14;

        uint8_t :8;
    };


    struct R8_Audio2VibeMaxmalOutputDrive:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x15;

        uint8_t :8;
    };

    struct R8_RatedVoltage:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x16;

        uint8_t :8;
    };

    struct R8_OverdriveClampVoltage:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x17;

        uint8_t :8;
    };

    struct R8_AutoCaliCompResult:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x18;

        uint8_t :8;
    };

    struct R8_AutoCaliBackEMF:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x19;

        uint8_t :8;
    };



    struct R8_FeedbackControl:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1a;

        uint8_t bemf_gain:2;
        uint8_t loop_gain:2;
        uint8_t fb_brake_factor:3;

        // 0: ERM Mode
        // 1: LRA Mode
        uint8_t n_erm_lra:1;
    };

    struct R8_Control1:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1b;

        uint8_t drive_time:5;
        uint8_t ac_couple:1;
        uint8_t :1;
        uint8_t startup_boost:1;
    };

    struct R8_Control2:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1c;

        uint8_t idiss_time:2;
        uint8_t blanking_time:2;
        uint8_t sample_time:2;
        uint8_t brake_stabilizer:2;
        uint8_t bidir_input:2;
    };

    struct R8_Control3:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1d;

        uint8_t lra_openloop:1;
        uint8_t n_pwm_analog:1;
        uint8_t lra_drive_mode:1;
        uint8_t data_format_rtp:1;
        uint8_t supply_comp_dis:1;
        uint8_t erm_openloop:1;
        uint8_t ng_thresh:1;
    };

    struct R8_Control4:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1e;

        uint8_t otp_program:1;
        uint8_t :1;
        uint8_t otp_status:1;
        uint8_t :1;
        uint8_t autocal_time:2;
        uint8_t :2;
    };

    struct R8_VbatVoltageMonitor:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1e;

        uint8_t :8;
    };

    struct R8_LRA_ResonancePeriod:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x1f;

        uint8_t :8;
    };
    
    R8_Status status_reg = {};
    R8_Mode mode_reg = {};
    R8_Rtp rtp_reg = {};
    R8_LibrarySel library_sel_reg = {};
    R8_WaveformSeg waveform_seg_reg = {};
    R8_Go go_reg = {};
    R8_OverrideTimeOffset override_time_offset_reg = {};
    R8_SustainTimeOffsetPositive sustain_time_offset_positive_reg = {};
    R8_SustainTimeOffsetNegitive sustain_time_offset_negitive_reg = {};
    R8_BrakeTimeOffset brake_time_offset_reg = {};
    R8_Audio2Vibe audio2vibe_reg = {};
    R8_Audio2VibeMinimalInputLevel audio2vibe_minimal_input_level_reg = {};
    R8_Audio2VibeMaxmalInputLevel audio2vibe_maxmal_input_level_reg = {};
    R8_Audio2VibeMinimalOutputDrive audio2vibe_minimal_output_drive_reg = {};
    R8_Audio2VibeMaxmalOutputDrive audio2vibe_maxmal_output_drive_reg = {};
    R8_RatedVoltage rated_voltage_reg = {};
    R8_OverdriveClampVoltage overdrive_clamp_voltage_reg = {};
    R8_AutoCaliCompResult auto_cali_comp_result_reg = {};
    R8_AutoCaliBackEMF auto_cali_back_emf_reg = {};
    R8_FeedbackControl feedback_control_reg = {};
    R8_Control1 control1_reg = {};
    R8_Control2 control2_reg = {};

};


class DRV2605L:public DRV2605L_Regs{
public:
    explicit DRV2605L(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit DRV2605L(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit DRV2605L(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> init();
    Package get_package();
    [[nodiscard]] IResult<> set_bemf_gain(const BemfGain gain);
    [[nodiscard]] IResult<> set_loop_gain(const LoopGain gain);
    [[nodiscard]] IResult<> play(const uint8_t idx);
    [[nodiscard]] IResult<> autocal();

private:
    hal::I2cDrv i2c_drv_;

    [[nodiscard]] IResult<> 
    write_reg(const RegAddr address, const uint8_t reg){
        if(const auto res = i2c_drv_.write_reg<uint8_t>(uint8_t(address), reg);
            res.is_err()) return Err(Error(res.unwrap_err()));

        return Ok();
    }

    [[nodiscard]] IResult<> 
    read_reg(const RegAddr addr, uint8_t & reg){
        const auto res = i2c_drv_.read_reg<uint8_t>(uint8_t(addr), reg);
        if(res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok();
    }

    [[nodiscard]] IResult<> 
    requestBurst(const RegAddr addr, uint8_t * data, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(data, len));
            res.is_err()) return Err(Error(res.unwrap_err()));
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.as_val());
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }
    [[nodiscard]] IResult<> read_reg(auto & reg){
        return read_reg(reg.address, reg.as_ref());
    }


    [[nodiscard]] IResult<> set_fb_brake_factor(const FbBrakeFactor factor);
    [[nodiscard]] IResult<> set_fb_brake_factor(const int fractor);
};

}