#pragma once

#include "robots/foc/motor_utils.hpp"

namespace ymd::foc{
using namespace ymd::drivers;
class ToneTasker{
protected:
    enum class Tones:uint16_t{
        N = 0,
        G4 = 392,
        A4 = 440,
        B4 = 494,
        C5 = 523,
        D5 = 587,
        E5 = 659,
        F5 = 698,
        G5 = 784
    };
    
    struct Tone{
        Tones freq_hz;
        Milliseconds sustain_ms;
    };

    static constexpr real_t tone_current = real_t(0.15);


    using enum Tones;
    
    static constexpr auto tones = std::to_array<Tone>({
        // {.freq_hz = A4,.sustain_ms = 100},  // 6
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = N,.sustain_ms = 20},
        // {.freq_hz = A4,.sustain_ms = 100},  // 6
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = N,.sustain_ms = 20},

        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = G5,.sustain_ms = 100},  // 5
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        
        // {.freq_hz = A4,.sustain_ms = 100},  // 6
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = N,.sustain_ms = 100},
        // {.freq_hz = A4,.sustain_ms = 100},  // 6
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = G5,.sustain_ms = 100},  // 5
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        
        // {.freq_hz = A4,.sustain_ms = 100},  // 6
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = G5,.sustain_ms = 100},  // 5
        // {.freq_hz = E5,.sustain_ms = 100},  // 3
        // {.freq_hz = D5,.sustain_ms = 100},  // 2
        
        {.freq_hz = B4, .sustain_ms = 100ms},  // 7
        {.freq_hz = C5, .sustain_ms = 100ms},  // 1
        {.freq_hz = B4, .sustain_ms = 100ms},  // 7
        {.freq_hz = G4, .sustain_ms = 100ms},  // 5
    });


    uint32_t cnt;
    uint32_t tone_index;
    Milliseconds play_begin_ms;
    
    digipw::SVPWM & svpwm_;
    bool done_ = false;
    
public:
    ToneTasker(digipw::SVPWM & _svpwm):svpwm_(_svpwm){;}

    void run();
    void reset();
    bool done();
};

}