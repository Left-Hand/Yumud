#include "robots/foc/stepper/stepper.hpp"

FOCStepper::RunStatus FOCStepper::beep_task(const FOCStepper::InitFlag init_flag){
    struct Tone{
        uint freq_hz;
        uint sustain_ms;
    };

    [[maybe_unused]] constexpr uint G4 = 392;
    [[maybe_unused]] constexpr uint A4 = 440;
    [[maybe_unused]] constexpr uint B4 = 494;
    [[maybe_unused]] constexpr uint C5 = 523;
    [[maybe_unused]] constexpr uint D5 = 587;
    [[maybe_unused]] constexpr uint E5 = 659;
    [[maybe_unused]] constexpr uint F5 = 698;
    [[maybe_unused]] constexpr uint G5 = 784;

    static const auto tones = std::to_array<Tone>({
        // {A4, 100},  // 6
        // {D5, 100},  // 2
        // {E5, 100},  // 3
        // {G5, 100},  // 5
        // {E5, 100},  // 3
        // {D5, 100},  // 2
        
        // {A4, 100},  // 6
        // {D5, 100},  // 2
        // {E5, 100},  // 3
        // {G5, 100},  // 5
        // {E5, 100},  // 3
        // {D5, 100},  // 2
        
        {B4, 100},  // 7
        {C5, 100},  // 1
        {B4, 100},  // 7
        {G4, 100},  // 5
    });

    static constexpr real_t tone_current = real_t(0.55);

    static uint32_t cnt;
    static uint32_t tone_index;
    static uint32_t play_begin_ms;

    if(init_flag){
        cnt = 0;
        tone_index = 0;
        play_begin_ms = millis();
        run_status = RunStatus::BEEP;
        return RunStatus::NONE;
    }

    if(millis() >= tones[tone_index].sustain_ms + play_begin_ms){ // play one note done
        if(tone_index >= tones.size()) return RunStatus::EXIT; // play done
        else{
            tone_index++;
            play_begin_ms = millis();
        }
    }
    
    {
        const auto & tone = tones[tone_index];
        const auto tone_period = foc_freq / tone.freq_hz;
        auto phase = sin(real_t(cnt % tone_period) / (tone_period) * real_t(TAU)) * real_t(PI/2);
        svpwm.setDuty(tone_current, phase);
        cnt++;
    }

    return RunStatus::NONE;
}