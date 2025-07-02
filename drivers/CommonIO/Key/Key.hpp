#pragma once

#include "KeyTrait.hpp"
#include "dsp/filter/homebrew/DigitalFilter.hpp"

namespace ymd::drivers{

class Key final:public KeyIntf{

public:
    Key(
        hal::GpioIntf & gpio, 
        const BoolLevel level
    ):gpio_(gpio), level_(level){;}

    void init(){
        init(level_);
    }

    void init(const BoolLevel level){
        if(level == HIGH){
            gpio_.inpd();
        }else{
            gpio_.inpu();
        }
    }

    void update() {
        last_state = now_state;
        filter_.update(gpio_.read().to_bool());
        now_state = filter_.result();
    }

    bool just_pressed() const {
        return last_state == false and now_state == true;
    }

    bool is_pressed() const {
        return now_state == true;
    }

    hal::GpioIntf & io(){
        return gpio_;
    }

private:
    hal::GpioIntf & gpio_;

    dsp::DigitalFilter filter_;
    const BoolLevel level_= LOW;

    bool last_state = false;
    bool now_state = false;
};


}