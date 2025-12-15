#pragma once

#include "gamepad.hpp"
#include "core/math/real.hpp"

namespace ymd::hid{

template<typename T>
struct [[nodiscard]] AxisInput final{
public:
    GamepadAxis axis;
    iq16 value;
};

}
