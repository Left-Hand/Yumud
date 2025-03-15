#pragma once

#include "drivers/device_defs.h"

#if     defined(DDS_MAX_PRECISION)
#if     !defined(MAX_U64)
#define MAX_U64 ((uint64_t)~0LL)
#endif
#endif

namespace ymd::drivers{


class AD9959{
// protected:
public:
    enum class ChannelIndex:uint8_t{
        ChannelNone = 0x00,
        Channel0    = 0x10,
        Channel1    = 0x20,
        Channel2    = 0x40,
        Channel3    = 0x80,
        ChannelAll  = 0xF0,
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
    ChannelIndex               last_channels;
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

    void setClock( int mult = 20,const int32_t calibration = 0); // Mult must be 0 or in range 4..20

    // Calculating deltas is expensive. You might use this infrequently and then use setDelta
    uint32_t frequencyDelta(uint32_t freq) const;

    void setFrequency(ChannelIndex chan, uint32_t freq);

    void setDelta(ChannelIndex chan, uint32_t delta);

    void setAmplitude(ChannelIndex chan, uint16_t amplitude);        // Maximum amplitude value is 1024

    void setPhase(ChannelIndex chan, uint16_t phase);                // Maximum phase value is 16383

    void update();

    void sweepFrequency(ChannelIndex chan, uint32_t freq, bool follow = true);      // Target frequency

    void sweepDelta(ChannelIndex chan, uint32_t delta, bool follow = true);

    void sweepAmplitude(ChannelIndex chan, uint16_t amplitude, bool follow = true);  // Target amplitude (half)

    void sweepPhase(ChannelIndex chan, uint16_t phase, bool follow = true);          // Target phase (180 degrees)

    void sweepRates(ChannelIndex chan, uint32_t increment, uint8_t up_rate, uint32_t decrement = 0, uint8_t down_rate = 0);

    void setChannels(ChannelIndex chan);
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
    uint32_t write(Register reg, uint32_t value)
    {
        // The indices of this array match the values of the Register enum:
        scexpr uint8_t register_length[8] = { 1, 3, 2, 3, 4, 2, 3, 2 };  // And 4 beyond that

        uint32_t    rval = 0;
        int         len = (uint8_t(reg)&0x7F) < sizeof(register_length)/sizeof(uint8_t) ? register_length[uint8_t(reg)&0x07] : 4;
        spi_drv_.writeSingle(uint8_t(reg)).unwrap();
        while (len-- > 0){
            uint8_t ret = 0;
            auto err = spi_drv_.transferSingle<uint8_t>(ret, (value>>len*8) & 0xFF);
            if(err.wrong()) return 0;
            rval = (rval<<8) | ret; 
        }
        return rval;
    }

};

}