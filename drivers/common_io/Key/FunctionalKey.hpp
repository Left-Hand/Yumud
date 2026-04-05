#pragma once

#include "Key.hpp"

namespace ymd::drivers{

class FunctionalKey:public KeyTrait{
public:
    enum class Event{

    };
protected:
    using Callback = std::function<void(bool)>;

    Callback long_press_cb_;
    Callback double_click_cb_;

    Key key_;
public:
    FunctionalKey(hal::GpioIntf & gpio, const Level _level):key_{gpio, _level}{;}

    bool pressed(){
        return key_.pressed();
    }
    hal::GpioIntf & io() override{
        return key_.io();
    }

    void update(){
        key_.update();
    }
}

}