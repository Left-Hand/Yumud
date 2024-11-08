#pragma once

#include "drivers/device_defs.h"

// #define DRV2605L_DEBUG

#ifdef DRV2605L_DEBUG
#undef DRV2605L_DEBUG
#define DRV2605L_DEBUG(...) DEBUG_PRINTLN(SpecToken::Space, std::hex, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define DRV2605L_DEBUG(...)
#endif

namespace yumud::drivers{

class DRV2605L{
public:
    scexpr uint8_t default_i2c_addr = 0b01100000;

    DRV2605L(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    DRV2605L(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}
    DRV2605L(I2c & i2c, const uint8_t addr = default_i2c_addr):i2c_drv_(I2cDrv(i2c, addr)){;}

    void update();

    bool verify();

    void reset();

protected:
    using RegAddress = uint8_t;

    I2cDrv i2c_drv_;

    enum class Package:uint8_t{
        _DRV2605,
        _DRV2604,
        _DRV2604L,
        _DRV2605L
    };

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

    struct StatusReg:public Reg8{
        scexpr RegAddress address = 0x00;

        uint8_t oc_detect:1;
        uint8_t over_temp:1;
        uint8_t :1;
        uint8_t diag_result:1;
        uint8_t :1;
        uint8_t device_id;
    };

    struct ModeReg:public Reg8{
        scexpr RegAddress address = 0x01;

        uint8_t mode:3;
        uint8_t :3;
        uint8_t standby:1;
        uint8_t dev_reset:1;
    };

    struct RtpReg:public Reg8{
        scexpr RegAddress address = 0x02;

        uint8_t rtp_input:8;
    };

    struct LibrarySelReg:public Reg8{
        scexpr RegAddress address = 0x03;

        uint8_t lib_sel:3;
        uint8_t :1;
        uint8_t hiz:1;
        uint8_t :3;
    };

    struct WaveformSegReg:public Reg8{
        scexpr RegAddress address = 0x04;
        scexpr RegAddress address_end = 0x0B;

        uint8_t wav_frm_seq:7;
        uint8_t wait:1;
    };

    struct GoReg:public Reg8{
        scexpr RegAddress address = 0x0C;

        uint8_t go:1;
        uint8_t :7;
    };

    struct OverrideTimeOffset:public Reg8{
        scexpr RegAddress address = 0x0D;

        uint8_t odt:8;
    };

    struct SustainTimeOffsetPositiveReg:public Reg8{
        scexpr RegAddress address = 0x0E;

        uint8_t :8;
    };

    struct SustainTimeOffsetNegitiveReg:public Reg8{
        scexpr RegAddress address = 0x0F;

        uint8_t :8;
    };

    struct BrakeTimeReg:public Reg8{
        scexpr RegAddress address = 0x10;

        uint8_t :8;
    };
    
    void writeReg(const RegAddress address, const uint8_t reg){
        i2c_drv_.writeReg((uint8_t)address, reg, MSB);
    }

    void readReg(const RegAddress address, uint8_t & reg){
        i2c_drv_.readReg((uint8_t)address, reg, MSB);
    }

    void requestPool(const RegAddress addr, uint8_t * data, size_t len){
        i2c_drv_.readMulti((uint8_t)addr, data, len, MSB);
    }
public:

};

}