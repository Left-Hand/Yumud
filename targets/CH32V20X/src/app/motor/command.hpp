#pragma once


#include "core/math/real.hpp"
#include "core/math/float/bf16.hpp"

namespace ymd::robots{

struct SetPositionCommand{
    q16 position;
    q16 speed;
};


struct StrightForwardCommand{
    q16 delta_position;
};

struct StrightForwardUntilEndstopCommand{

};

struct SetSteerCommand{
    q16 steer;
};

struct SpinCommand{
    q16 delta_rotation;
};


struct NestU8Command{
    std::array<uint8_t, 8> buf;
};


struct SetPositionXYZCommand{
    q16 x;
    q16 y;
    q16 z;
};


struct SetKpKdCommand{
    q16 kp;
    q16 kd;
};

struct MoveCommand{
    bf16 x;
    bf16 y;
};

struct PressCommand{
    bf16 z;
};

struct ReleaseCommand{
    bf16 z;
};

struct ReplaceCommand{
    bf16 x1, y1;
    bf16 x2, y2;
};

struct AbortCommand{
    bf16 z;
};

using VCommand = std::variant<
    MoveCommand,
    PressCommand,
    ReleaseCommand,
    ReplaceCommand,
    AbortCommand
>;

}
