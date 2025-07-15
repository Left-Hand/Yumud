#pragma once


#include "core/math/real.hpp"
#include "core/math/float/bf16.hpp"

namespace ymd::robots{

namespace joint_cmds{


struct SetPosition{
    bf16 position;
};


struct SetSpeed{
    bf16 speed;
};


struct SetPositionAndSpeed{
    bf16 position;
    bf16 speed;
};


struct SetTrapzoid{
    bf16 position;
    bf16 speed;
    bf16 acceleration;
    bf16 deceleration;
};


}

namespace machine_cmds{

struct SetPositionXYZ{
    bf16 x;
    bf16 y;
    bf16 z;
};

struct SetPositionXY{
    bf16 x;
    bf16 y;
    bf16 z;
};

struct SetPolar{
    bf16 radius;
    bf16 phi;
};

struct StrightForward{
    bf16 delta_position;
};

struct StrightForwardUntilEndstop{

};

struct SetSteer{
    bf16 steer;
};

struct Spin{
    bf16 delta_rotation;
};


struct NestU8{
    std::array<uint8_t, 8> buf;
};


struct SetKpKd{
    bf16 kp;
    bf16 kd;
};

struct Move{
    bf16 x;
    bf16 y;
};

struct Press{
    bf16 z;
};

struct Release{
    bf16 z;
};

struct Replace{
    bf16 x1, y1;
    bf16 x2, y2;
};

struct Abort{
    bf16 z;
};

using V = std::variant<
    Move,
    Press,
    Release,
    Replace,
    Abort
>;

}
}