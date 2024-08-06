#include "../robots/stepper/stepper.hpp"
Stepper::RunStatus Stepper::beep_task(const Stepper::InitFlag init_flag){
    struct Tone{
        uint32_t freq_hz;
        uint32_t sustain_ms;
    };

    [[maybe_unused]] constexpr int freq_G4 = 392;
    [[maybe_unused]] constexpr int freq_A4 = 440;
    [[maybe_unused]] constexpr int freq_B4 = 494;
    [[maybe_unused]] constexpr int freq_C5 = 523;
    [[maybe_unused]] constexpr int freq_D5 = 587;
    [[maybe_unused]] constexpr int freq_E5 = 659;
    [[maybe_unused]] constexpr int freq_F5 = 698;
    [[maybe_unused]] constexpr int freq_G5 = 784;

    static const auto tones = std::to_array<Tone>({
        {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
        {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
        {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
        {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
        {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        
        {.freq_hz = freq_A4,.sustain_ms = 100},  // 6
        {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
        {.freq_hz = freq_G5,.sustain_ms = 100},  // 5
        {.freq_hz = freq_E5,.sustain_ms = 100},  // 3
        {.freq_hz = freq_D5,.sustain_ms = 100},  // 2
        
        {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
        {.freq_hz = freq_C5,.sustain_ms = 100},  // 1
        {.freq_hz = freq_B4,.sustain_ms = 100},  // 7
        {.freq_hz = freq_G4,.sustain_ms = 100},  // 5

    });

    static constexpr real_t tone_current = 0.7;

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
        auto phase = sin(real_t(cnt % tone_period) / (tone_period) * TAU);
        setCurrent(tone_current, phase * PI/2);
        cnt++;
    }

    return RunStatus::NONE;
}