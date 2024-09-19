#pragma once

#include "../constants.hpp"

class ToneTasker{
protected:
    enum class Tones:int{
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
        int sustain_ms;
    };


    scexpr real_t tone_current = real_t(0.55);
    


    using enum Tones;
    
    std::array<Tone, 4> tones = {
        Tone{B4, 100},  // 7
        Tone{C5, 100},  // 1
        Tone{B4, 100},  // 7
        Tone{G4, 100},  // 5
    };


    uint32_t cnt;
    uint32_t tone_index;
    uint32_t play_begin_ms;
    
    SVPWM & svpwm_;
    bool done_ = false;
    
public:
    ToneTasker(SVPWM & _svpwm):svpwm_(_svpwm){;}

    void run();
    void reset();
    bool done();
};