#include "robots/stepper/stepper.hpp"
Stepper::ExitFlag Stepper::beep_task(const Stepper::InitFlag init_flag){
    struct Tone{
        uint32_t freq_hz;
        uint32_t sustain_ms;
    };

    constexpr int freq_G4 = 392;
    constexpr int freq_A4 = 440;
    constexpr int freq_B4 = 494;
    constexpr int freq_C5 = 523;
    constexpr int freq_D5 = 587;
    constexpr int freq_E5 = 659;
    constexpr int freq_F5 = 698;
    constexpr int freq_G5 = 784;

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

        {.freq_hz = freq_F5,.sustain_ms = 100}   // 6
    });

    constexpr float tone_current = 0.4;

    static uint32_t cnt;
    static uint32_t tone_index;
    static uint32_t play_begin_ms;

    if(init_flag){
        cnt = 0;
        tone_index = 0;
        play_begin_ms = millis();
    }

    if(millis() >= tones[tone_index].sustain_ms + play_begin_ms){ // play one note done
        if(tone_index >= tones.size()) return true; // play done
        else{
            tone_index++;
            play_begin_ms = millis();
        }
    }
    
    {
        const auto & tone = tones[tone_index];
        auto tone_cnt = foc_freq / tone.freq_hz / 2;
        bool phase = (cnt / tone_cnt) % 2;
        setCurrent(real_t(tone_current), phase ? real_t(0.5) : real_t(-0.5));
        cnt++;
    }
    return false;
}