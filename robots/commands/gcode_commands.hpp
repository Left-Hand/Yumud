#pragma once

#include "core/utils/serde.hpp"

namespace ymd{



namespace robots::gcode_commands{

struct RapidMove{
    math::bf16 x;
    math::bf16 y;
};

struct LinearMove{
    math::bf16 x;
    math::bf16 y;
};

struct Dwell{
    uint16_t dwell_ms;
};

struct UseInchesUnits{

};

struct UseMillimetersUnits{

};

using G0 = RapidMove;
using G1 = LinearMove;
using G20 = UseInchesUnits;
using G21 = UseMillimetersUnits;
}
}