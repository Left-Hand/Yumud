#include "tone.hpp"

using namespace yumud::foc;

void ToneTasker::run(){
    using enum Tones;

    if(done_) return;

    if(millis() >= tones[tone_index].sustain_ms + play_begin_ms){ // play one note done
        if(tone_index >= tones.size()){
            done_ = true;
            return;
        } // play done
        else{
            tone_index++;
            play_begin_ms = millis();
        }
    }
    
    {
        const auto & tone = tones[tone_index];
        const auto tone_period = foc_freq / int(tone.freq_hz);
        auto phase = sin(real_t(cnt % tone_period) / (tone_period) * real_t(TAU)) * real_t(PI/2);
        svpwm_.setDuty(tone_current, phase);
        cnt++;
    }
}

void ToneTasker::reset(){
    cnt = 0;
    done_ = false;
    tone_index = 0;
    play_begin_ms = millis();
}

bool ToneTasker::done(){
    return done_;
}