#pragma once

#include <algorithm>
#include <array>
#include <tuple>


#include "core/math/real.hpp"
#include "core/utils/Result.hpp"
#include "config.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"

namespace ymd{

struct LapPosition{
    real_t position;
};

struct RawLapPosition{
    real_t position;
};

struct Elecrad{
    real_t elecrad;
};



}