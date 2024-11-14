#pragma once

#include "KeyTrait.hpp"
#include "dsp/filter/DigitalFilter.hpp"

namespace ymd::drivers{

class Key:public KeyTrait{
protected:
    using Level = BoolLevel;

    DigitalFilter filter;
    GpioConcept & m_gpio;
    const Level level_= LOW;

    bool last_state = false;
    bool now_state = false;
public:
    Key(GpioConcept & gpio, const Level _level):m_gpio(gpio), level_(_level){;}

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

    void update() override {
        last_state = now_state;
        now_state = filter.update(bool(*this));
    }

    bool pressed() override{
        return last_state == false and now_state == true;
    }

    operator bool() const {
        return m_gpio.read() == bool(level_);
    }

    GpioConcept & io() override{
        return m_gpio;
    }
};


}