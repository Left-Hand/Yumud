#pragma once

#include "keycode.hpp"
#include "sstl/include/sstl/vector.h"

namespace ymd::hid{


template<typename T>
class ButtonInput final{
};




template<>
class ButtonInput<KeyCode>{
    constexpr bool pressed() const {
        return pressed_;
    }

    constexpr bool just_pressed() const {
        return pressed_ && !last_pressed_;
    }

    constexpr bool just_released() const {
        return !pressed_ && last_pressed_;
    }
private:
    KeyCode key_;
    bool pressed_;
    bool last_pressed_;
};

}