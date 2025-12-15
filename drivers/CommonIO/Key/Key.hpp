#pragma once

#include "KeyTrait.hpp"
#include "dsp/filter/homebrew/debounce_filter.hpp"

namespace ymd::drivers{

struct Key final{

public:
    struct Config{
        hal::Gpio & gpio;
        BoolLevel act_level;
		uint8_t threshold = 2;
		uint8_t pipe_length = 8;
    };

    explicit Key(
        const Config & cfg
    ):
        gpio_(cfg.gpio), 
        level_(cfg.act_level),
        filter_(typename dsp::DebounceFilter::Config{
            .pipe_length = cfg.pipe_length,
            .threshold = cfg.threshold
        }){;}


    void update() {
        last_state = now_state;
        filter_.update(gpio_.read() == level_);
        now_state = filter_.is_active();
    }

    [[nodiscard]] bool just_pressed() const {
        return last_state == false and now_state == true;
    }

    [[nodiscard]] bool is_pressed() const {
        return now_state == true;
    }

    hal::Gpio & io(){
        return gpio_;
    }

private:
    hal::Gpio & gpio_;
    BoolLevel level_;
    dsp::DebounceFilter filter_;

    bool last_state = false;
    bool now_state = false;
};


}