#include "ad9959.hpp"


using namespace ymd::drivers;
using namespace ymd;


void AD9959::init(){
    core_clock = 0;
    last_channels = ChannelIndex::ChannelAll;
    // Ensure that the SPI device is initialised
    // "setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high"

    // digitalWrite(ResetPin, 0);

    // ResetPin = false;
    reset_gpio.outpp(LOW);
    // pinMode(ResetPin, OUTPUT);          // Ensure we can reset the AD9959
    // digitalWrite(ChipEnablePin, 1);

    // pinMode(ChipEnablePin, OUTPUT);     // This control signal applies the loaded values
    // digitalWrite(UpdatePin, 0);
    update_gpio.outpp(LOW);
    // pinMode(UpdatePin, OUTPUT);         // This control signal applies the loaded values

    reset();
}

    /*
    * Reset, applying CFR bits requested.
    * You might want to add:
    * CFR_Bits::AutoclearSweep - clear the sweep accumulator on I/O update or profile change
    * CFR_Bits::AutoclearPhase - clear the phase accumulator on I/O update or profile change
    */
void AD9959::reset(CFR_Bits cfr){
    pulse(reset_gpio);                    // (minimum 5 cycles of the 30MHz clock)
    pulse(update_gpio);

    // Apply the requested CFR bits
    last_channels = ChannelIndex::ChannelNone;        // Ensure channels get set, not optimised out
    setChannels(ChannelIndex::ChannelAll);
    write(Register::CFR, uint8_t(cfr));

    setChannels(ChannelIndex::ChannelNone);           // Disable all channels, set 3-wire MSB mode:
    pulse(update_gpio);                   // Apply the changes
    setClock();                         // Set the PLL going
    // It will take up to a millisecond before the PLL locks and stabilises.
}

void AD9959::setClock( int mult,const int32_t calibration) // Mult must be 0 or in range 4..20
{
    if (mult < 4 || mult > 20)
        mult = 1;                         // Multiplier is disabled.
    core_clock = reference_freq * (1000000000ULL+calibration) / 1000000000ULL * mult;
    #if defined(DDS_MAX_PRECISION)
    reciprocal = MAX_U64 / core_clock;
    #else
    // The AVR gcc implementation has a 32x32->64 widening multiply.
    // This is quite accurate enough, and considerably faster than full 64x64.
    uint64_t    scaled = core_clock;
    for (shift = 32; shift > 0 && (scaled&0x100000000ULL) == 0; shift--)
        scaled <<= 1;                   // Ensure that reciprocal fits in 32 bits
    reciprocal = (0x1ULL<<(32+shift)) / core_clock;
    #endif
    // Serial.print("core_clock="); Serial.println(core_clock);
    // Serial.print("reciprocal="); Serial.println(reciprocal);



    spi_drv_.writeSingle(Register::FR1);
    // High VCO Gain is needed for a 255-500MHz master clock, and not up to 160Mhz
    // In-between is unspecified.
    spi_drv_.writeSingle(
        (core_clock > 200 ? uint8_t(FR1_Bits::VCOGain) : uint8_t(0)) |
        (mult*uint8_t(FR1_Bits::PllDivider)) | 
        uint8_t(FR1_Bits::ChargePump3)         // Lock fast
    );
    // Profile0 means each channel is modulated by a different profile pin:
    spi_drv_.writeSingle(
        uint8_t(FR1_Bits::ModLevels2) |
        uint8_t(FR1_Bits::RampUpDownOff) |
        uint8_t(FR1_Bits::Profile0));
    spi_drv_.writeSingle(FR1_Bits::SyncClkDisable); // Don't output SYNC_CLK
}

    // Calculating deltas is expensive. You might use this infrequently and then use setDelta
uint32_t AD9959::frequencyDelta(uint32_t freq) const{
    #if defined(DDS_MAX_PRECISION)
    return (freq * reciprocal + 0x80000000UL) >> 32;
    #else
    // The reciprocal/16 is a rounding factor determined experimentally
    return ((uint64_t)freq * reciprocal + reciprocal/16) >> shift;
    #endif
}

void AD9959::setFrequency(ChannelIndex chan, uint32_t freq){
    setDelta(chan, frequencyDelta(freq));
}

void AD9959::setDelta(ChannelIndex chan, uint32_t delta){
    setChannels(chan);
    write(Register::CFTW, delta);
}

void AD9959::setAmplitude(ChannelIndex chan, uint16_t amplitude){        // Maximum amplitude value is 1024

    if (amplitude > 1024)
        amplitude = 1024;                 // Clamp to the maximum
    setChannels(chan);
    spi_drv_.writeSingle(Register::ACR);                  // Amplitude control register
    spi_drv_.writeSingle(0);                    // Time between ramp steps
    if (amplitude < 1024){               // Enable amplitude control with no ramping
        spi_drv_.writeSingle((uint16_t(ACR_Bits::MultiplierEnable) | amplitude)>>8);
    }else{
        spi_drv_.writeSingle(0);                  // Disable the amplitude multiplier
    }
    spi_drv_.writeSingle(amplitude&0xFF);       // Bottom 8 bits of amplitude
}

void AD9959::setPhase(ChannelIndex chan, uint16_t phase){                // Maximum phase value is 16383
    setChannels(chan);
    write(Register::CPOW, phase & 0x3FFF);        // Phase wraps around anyway
}

void AD9959::update(){
    pulse(update_gpio);
}

void AD9959::sweepFrequency(ChannelIndex chan, uint32_t freq, bool follow){       // Target frequency
    sweepDelta(chan, frequencyDelta(freq), follow);
}

void AD9959::sweepDelta(ChannelIndex chan, uint32_t delta, bool follow){
    setChannels(chan);
    // Set up for frequency sweep
    write(
        Register::CFR,
        uint8_t(CFR_Bits::FrequencyModulation) |
        uint8_t(CFR_Bits::SweepEnable) |
        uint8_t(CFR_Bits::DACFullScale) |
        uint8_t(CFR_Bits::MatchPipeDelay) |
        (follow ? uint8_t(0) : uint8_t(CFR_Bits::SweepNoDwell))
    );
    // Write the frequency delta into the sweep destination register
    write(Register::CW1, delta);
}

void AD9959::sweepAmplitude(ChannelIndex chan, uint16_t amplitude, bool follow){  // Target amplitude (half)
    setChannels(chan);

    // Set up for amplitude sweep
    write(
        Register::CFR,
        uint8_t(CFR_Bits::AmplitudeModulation) |
        uint8_t(CFR_Bits::SweepEnable) |
        uint8_t(CFR_Bits::DACFullScale) |
        uint8_t(CFR_Bits::MatchPipeDelay) |
        (follow ? uint8_t(0) : uint8_t(CFR_Bits::SweepNoDwell))
    );

    // Write the amplitude into the sweep destination register, MSB aligned
    write(Register::CW1, ((uint32_t)amplitude) * (0x1<<(32-10)));
}

void AD9959::sweepPhase(ChannelIndex chan, uint16_t phase, bool follow){          // Target phase (180 degrees)
    setChannels(chan);

    // Set up for phase sweep
    write(
        Register::CFR,
        uint8_t(CFR_Bits::PhaseModulation) |
        uint8_t(CFR_Bits::SweepEnable) |
        uint8_t(CFR_Bits::DACFullScale) |
        uint8_t(CFR_Bits::MatchPipeDelay) |
        (follow ? uint8_t(0) : uint8_t(CFR_Bits::SweepNoDwell))
    );

    // Write the phase into the sweep destination register, MSB aligned
    write(Register::CW1, ((uint32_t)phase) * (0x1<<(32-14)));
}

void AD9959::sweepRates(ChannelIndex chan, uint32_t increment, uint8_t up_rate, uint32_t decrement, uint8_t down_rate){

    setChannels(chan);
    write(Register::RDW, increment);                      // Rising Sweep Delta Word
    write(Register::FDW, increment);                      // Falling Sweep Delta Word
    write(Register::LSRR, (down_rate<<8) | up_rate);      // Linear Sweep Ramp Rate
}

void AD9959::setChannels(ChannelIndex chan){
    if (last_channels != chan){
        write(Register::CSR, (uint8_t)chan|(uint8_t)CSR_Bits::MSB_First|(uint8_t)CSR_Bits::IO3Wire);
        last_channels = chan;
    }
}    // To read channel registers, you must first use setChannels to select exactly one channel!
uint32_t AD9959::read(Register reg){
    return write(Register(0x80|uint8_t(reg)), 0);  // The zero data is discarded, just the return value is used
}