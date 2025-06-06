#pragma once

#include <cstdint>

namespace ymd::hid{
enum class GamepadAxis:uint8_t{
    LeftStickX,
    LeftStickY,
    LeftZ,
    RightStickX,
    RightStickY,
    RightZ,
};

}