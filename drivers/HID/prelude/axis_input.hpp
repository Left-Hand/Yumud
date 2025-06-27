#pragma once

#include "gamepad.hpp"

namespace ymd::hid{

template<typename T>
class AxisInput final{
public:
    using Axis = GamepadAxis;
    constexpr AxisInput(Axis axis, real_t value)
    : axis_(axis),
        value_(value) {}

    constexpr Axis axis() const {
        return axis_;
    }

    constexpr real_t value() const {
        return value_;
    }

private:


    Axis axis_;
    real_t value_;
};

}
