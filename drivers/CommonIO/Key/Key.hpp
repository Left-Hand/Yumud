#pragma once

#include "KeyTrait.hpp"
#include "dsp/filter/homebrew/DigitalFilter.hpp"

namespace ymd::drivers{

class Key final:public KeyIntf{
protected:
    using Level = BoolLevel;
    hal::GpioIntf & m_gpio;

    dsp::DigitalFilter filter;
    const Level level_= LOW;

    bool last_state = false;
    bool now_state = false;
public:
    Key(hal::GpioIntf & gpio, const Level _level):m_gpio(gpio), level_(_level){;}

    void init(){
        init(level_);
    }

    void init(const Level level){
        if(level){
            m_gpio.inpd();
        }else{
            m_gpio.inpu();
        }
    }

    void update() {
        last_state = now_state;
        filter.update(bool(m_gpio.read()));
        now_state = filter.result();
    }

    bool just_pressed() const {
        return last_state == false and now_state == true;
    }

    bool is_pressed() const {
        return now_state == true;
    }

    hal::GpioIntf & io(){
        return m_gpio;
    }
};


}