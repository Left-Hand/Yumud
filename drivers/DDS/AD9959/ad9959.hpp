#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"

namespace ymd::drivers{

// REG_TEMPLATE(Reg24, uint24_t)
struct AD9959_Regs{
    using RegAddress = uint8_t;
    struct R8_ChannelSelect:public Reg8<>{
        scexpr RegAddress address = 0x00;

        uint8_t lsb_first:1;
        uint8_t serial_io_mode:2;
        uint8_t __resv__:1;
        uint8_t channel0_en:1;
        uint8_t channel1_en:1;
        uint8_t channel2_en:1;
        uint8_t channel3_en:1;
    } DEF_R8(channel_select)

    // struct R32_Funtion1:public Reg32<>{
    //     scexpr RegAddress address = 0x01;

    //     uint8_t manual_software_sync:1;
    //     uint8_t manual_hardware_sync:1;
    //     uint8_t open:2;
    //     uint8_t dac_rederence_power_down:1;
    //     uint8_t sync_clk_disen:1;
    //     uint8_t external_power_down_mode:1;
    //     uint8_t ref_clock_power_down:1;
    //     uint8_t modu_level:1;
    //     uint8_t ru_rd:1;
    //     uint8_t pin_conf:3;
    //     uint8_t open2:1;
    //     uint8_t charge_pump_ctrl:2;
    //     uint8_t pll_divider_ratio:5;
    //     uint8_t vco_gain_ctrl:1;
    //     uint8_t __resv__;
    // } DEF_R32(function1);

    struct R16_Funtion2:public Reg16<>{
        scexpr RegAddress address = 0x02;

        uint8_t system_clock_offset:2;
        uint8_t open:2;
        uint8_t multidev_sync_mask:1;
        uint8_t multidev_sync_stat:1;
        uint8_t multidev_master_en:1;
        uint8_t auto_sync_en:1;
        uint8_t open2:2;
        uint8_t open3:2;
        uint8_t all_channel_clr_phase_accu:1;
        uint8_t all_channel_autoclr_phase_accu:1;
        uint8_t all_channel_clr_sweep_accu:1;
        uint8_t all_channel_autoclr_sweep_accu:1;
    } DEF_R16(function2);

    struct R32_ChannelFunction:public Reg32<>{
        scexpr RegAddress address = 0x03;

        uint8_t sinewave_output_en:1;
        uint8_t clr_phase_accu:1;
        uint8_t autoclr_phase_accu:1;
        uint8_t clear_sweep_accu:1;
        uint8_t autoclr_sweep_accu:1;
        uint8_t matched_pipe_delays_active:1;
        uint8_t dac_pwdn:1;
        uint8_t digital_pwdn:1;
        uint8_t dac_fs_current:2;
        uint8_t __zero__:1 = 0;
        uint8_t open:2;
        uint8_t load_ssr_at_io_update:1;
        uint8_t linear_sweep_en:1;
        uint8_t linear_sweep_nodwell:1;
        uint8_t open2:6;
        uint8_t afp_sel:2;
    };

    struct R32_ChannelFreqencyTuning:public Reg32<>{
        scexpr RegAddress address = 0x04;
        uint32_t data;
    };

    struct R32_AmplitudeControl:public Reg32<>{
        scexpr RegAddress address = 0x06;

        uint32_t factor:10;
        uint32_t load_addr_at_io_update:1;
        uint32_t rurd_en:1;
        uint8_t amp_multiplier_en:1;
        uint32_t open:1;
        uint32_t inc_dev_step_size:1;
        uint32_t ramp_rate:8;
        uint32_t :8;
    };



    struct R32_RisingDeltaWord:public Reg32<>{
        scexpr RegAddress address = 0x04;

        uint16_t data;
    };



    struct R16_LinearSweepRate:public Reg16<>{
        scexpr RegAddress address = 0x07;

        uint16_t data;
    };

    struct R32_FallingDeltaWord:public Reg32<>{
        scexpr RegAddress address = 0x08;

        uint32_t data;
    };

    struct R32_ChannelWord:public Reg32<>{
        scexpr RegAddress address = 0x09;
        uint32_t data;
    };

    std::array<R32_ChannelWord, 16> channel_words;
};

class AD9959{
protected:
    enum class ChannelNth:uint8_t{
        Nil = 0,
        _0    = 0x10,
        _1    = 0x20,
        _2    = 0x40,
        _3    = 0x80,
        All  = 0xF0,
    } ;

    // See register_length[] in write() before re-ordering these.
    enum class Register:uint8_t{        // There are 334 bytes in all the registers! See why below...
        CSR               = 0x00,   // 1 byte, Channel Select Register
        FR1               = 0x01,   // 3 bytes, Function Register 1
        FR2               = 0x02,   // 2 bytes, Function Register 2
                                    // The following registers are duplicated for each channel.
                                    // A write goes to any and all registers enabled in channel select (CSR)
                                    // To read successfully you must first select one channel
        CFR               = 0x03,   // 3 bytes, Channel Function Register (one for each channel!)
        CFTW              = 0x04,   // 4 bytes, Channel Frequency Tuning Word
        CPOW              = 0x05,   // 2 bytes, Channel Phase Offset Word (aligned to LSB, top 2 bits unused)
        ACR               = 0x06,   // 3 bytes, Amplitude Control Register (rate byte, control byte, scale byte)
        LSRR              = 0x07,   // 2 bytes, Linear Sweep Rate Register (falling, rising)
        RDW               = 0x08,   // 4 bytes, Rising Delta Word
        FDW               = 0x09,   // 4 bytes, Falling Delta Word
                                    // The following registers (per channel) are used to provide 16 modulation values
                                    // This library doesn't provide modulation. Only CW1 is used, for sweep destination.
        CW1               = 0x0A,   // 4 bytes, Channel Word 1-15 (phase & amplitude MSB aligned)
        CW2               = 0x0B,
        CW3               = 0x0C,
        CW4               = 0x0D,
        CW5               = 0x0E,
        CW6               = 0x0F,
        CW7               = 0x10,
        CW8               = 0x11,
        CW9               = 0x12,
        CW10              = 0x13,
        CW11              = 0x14,
        CW12              = 0x15,
        CW13              = 0x16,
        CW14              = 0x17,
        CW15              = 0x18
    };

    enum class CSR_Bits:uint8_t{
        // Bit order selection (default MSB):
        MSB_First = 0x00,
        LSB_First = 0x01,
        // Serial I/O Modes (default IO2Wire):
        IO2Wire = 0x00,
        IO3Wire = 0x02,
        IO2Bit = 0x04,
        IO4Bit = 0x06,
    } ;

    enum class FR1_Bits:uint8_t{    // Function Register 1 is 3 bytes wide.
        // Most significant byte:
        // Higher charge pump values decrease lock time and increase phase noise
        ChargePump0      = 0x00,
        ChargePump1      = 0x01,
        ChargePump2      = 0x02,
        ChargePump3      = 0x03,

        PllDivider       = 0x04,    // multiply 4..20 by this (or shift 19)
        VCOGain          = 0x80,    // Set low for VCO<160MHz, high for >255MHz

        // Middle byte:
        ModLevels2       = 0x00,    // How many levels of modulation?
        ModLevels4       = 0x01,
        ModLevels8       = 0x02,
        ModLevels16      = 0x03,

        RampUpDownOff    = 0x00,    // Which pins contol amplitude ramping?
        RampUpDownP2P3   = 0x04,    // Profile=0 means ramp-up, 1 means ramp-down
        RampUpDownP3     = 0x08,    // Profile=0 means ramp-up, 1 means ramp-down
        RampUpDownSDIO123= 0x0C,    // Only in 1-bit I/O mode

        Profile0         = 0x00,
        Profile7         = 0x07,

        // Least significant byte:
        SyncAuto         = 0x00,    // Master SYNC_OUT->Slave SYNC_IN, with FR2
        SyncSoft         = 0x01,    // Each time this is set, system clock slips one cycle
        SyncHard         = 0x02,    // Synchronise devices by slipping on SYNC_IN signal

        // Software can power-down individual channels (using CFR[7:6])
        DACRefPwrDown    = 0x10,    // Power-down DAC reference
        SyncClkDisable   = 0x20,    // Don't output SYNC_CLK
        ExtFullPwrDown   = 0x40,    // External power-down means full power-down (DAC&PLL)
        RefClkInPwrDown  = 0x80,    // Disable reference clock input
    } ;

    enum class FR2_Bits:uint16_t{
        AllChanAutoClearSweep    = 0x8000,  // Clear sweep accumulator(s) on I/O_UPDATE
        AllChanClearSweep        = 0x4000,  // Clear sweep accumulator(s) immediately
        AllChanAutoClearPhase    = 0x2000,  // Clear phase accumulator(s) on I/O_UPDATE
        AllChanClearPhase        = 0x2000,  // Clear phase accumulator(s) immediately
        AutoSyncEnable   = 0x0080,
        MasterSyncEnable = 0x0040,
        MasterSyncStatus = 0x0020,
        MasterSyncMask   = 0x0010,
        SystemClockOffset = 0x0003,         // Mask for 2-bit clock offset controls
    };

    // Channel Function Register
    enum class CFR_Bits:uint32_t{
        ModulationMode   = 0xC00000,        // Mask for modulation mode
        AmplitudeModulation = 0x400000,     // Mask for modulation mode
        FrequencyModulation = 0x800000,     // Mask for modulation mode
        PhaseModulation  = 0xC00000,        // Mask for modulation mode
        SweepNoDwell     = 0x008000,        // No dwell mode
        SweepEnable      = 0x004000,        // Enable the sweep
        SweepStepTimerExt = 0x002000,       // Reset the sweep step timer on I/O_UPDATE
        DACFullScale     = 0x000300,        // 1/8, 1/4, 1/2 or full DAC current
        DigitalPowerDown = 0x000080,        // Power down the DDS core
        DACPowerDown     = 0x000040,        // Power down the DAC
        MatchPipeDelay   = 0x000020,        // Compensate for pipeline delays
        AutoclearSweep   = 0x000010,        // Clear the sweep accumulator on I/O_UPDATE
        ClearSweep       = 0x000008,        // Clear the sweep accumulator immediately
        AutoclearPhase   = 0x000004,        // Clear the phase accumulator on I/O_UPDATE
        ClearPhase       = 0x000002,        // Clear the phase accumulator immediately
        OutputSineWave   = 0x000001,        // default is cosine
    };

    // Amplitude Control Register
    enum class ACR_Bits:uint32_t{
        RampRate            = 0xFF0000,     // Time between ramp steps
        StepSize            = 0x00C000,     // Amplitude step size (00=1,01=2,10=4,11=8)
        MultiplierEnable    = 0x001000,     // 0 means bypass the amplitude multiplier
        RampEnable          = 0x000800,     // 0 means aplitude control is manual
        LoadARRAtIOUpdate   = 0x000400,     // Reload Amplitude Rate Register at I/O Update
        ScaleFactor         = 0x0003FF,     // 10 bits for the amplitude target
    } ;

protected:
    uint32_t              core_clock;             // reference_freq*pll_mult after calibration
    #if     defined(DDS_MAX_PRECISION)
    uint64_t              reciprocal;             // (2^64-1)/core_clock
    #else
    uint32_t              reciprocal;             // 2^(64-shift)/core_clock
    uint8_t               shift;                  // (2<<shift) < core_clock, but just (28 or less)
    #endif
    ChannelNth               last_channels;
    scexpr uint32_t reference_freq = 25000000; // Use your crystal or reference frequency
    hal::SpiDrv spi_drv_;
    hal::GpioIntf &         reset_gpio;               // Reset pin (active = high)
    hal::GpioIntf &         update_gpio;              // I/O_UPDATE: Apply config changes

public:
    AD9959(
        const hal::SpiDrv & spi_drv, 
        hal::GpioIntf & _reset_gpio = hal::NullGpio, 
        hal::GpioIntf & _update_gpio = hal::NullGpio
    ):
        spi_drv_(spi_drv),
        reset_gpio(_reset_gpio),
        update_gpio(_update_gpio){;}

    AD9959(
        hal::SpiDrv && spi_drv, 
        hal::GpioIntf & _reset_gpio = hal::NullGpio, 
        hal::GpioIntf & _update_gpio = hal::NullGpio
    ):
        spi_drv_(std::move(spi_drv)),
        reset_gpio(_reset_gpio),
        update_gpio(_update_gpio){;}


    void init();

    void reset(CFR_Bits cfr = CFR_Bits(
            uint8_t(CFR_Bits::DACFullScale) | 
            uint8_t(CFR_Bits::MatchPipeDelay) |
            uint8_t(CFR_Bits::OutputSineWave)));

    void set_clock( int mult = 20,const int32_t calibration = 0); // Mult must be 0 or in range 4..20

    // Calculating deltas is expensive. You might use this infrequently and then use setDelta
    uint32_t frequency_delta(uint32_t freq) const;

    void set_frequency(ChannelNth chan, uint32_t freq);

    void set_delta(ChannelNth chan, uint32_t delta);

    void set_amplitude(ChannelNth chan, uint16_t amplitude);        // Maximum amplitude value is 1024

    void set_phase(ChannelNth chan, uint16_t phase);                // Maximum phase value is 16383

    void update();

    void sweep_frequency(ChannelNth chan, uint32_t freq, bool follow = true);      // Target frequency

    void sweep_delta(ChannelNth chan, uint32_t delta, bool follow = true);

    void sweep_amplitude(ChannelNth chan, uint16_t amplitude, bool follow = true);  // Target amplitude (half)

    void sweep_phase(ChannelNth chan, uint16_t phase, bool follow = true);          // Target phase (180 degrees)

    void sweep_rates(ChannelNth chan, uint32_t increment, uint8_t up_rate, uint32_t decrement = 0, uint8_t down_rate = 0);

    void set_channels(ChannelNth chan);
    // To read channel registers, you must first use setChannels to select exactly one channel!
    uint32_t read(Register reg);
    protected:
    void pulse(hal::GpioIntf & gpio){
        raise(gpio);
        lower(gpio);
    }

    void lower(hal::GpioIntf & gpio){
        gpio.clr();
    }

    void raise(hal::GpioIntf & gpio){
        gpio.set();
    }
    uint32_t write(Register reg, uint32_t value);

};

}