#pragma once

#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"
#include "body.hpp"

namespace SMC{

struct GlobalConfig{
    Grayscale positive_threshold = 60;
    Grayscale edge_threshold = 30;
    int safety_seed_height = 0;
    real_t dpv = 1.8;

    Rangei valid_width = Rangei(188 / 9, 188 / 4);
    int align_space_width = 12;
    bool align_right = true;
    uint8_t enable_flag = false;
    int frame_ms = 0;

    real_t dir_merge_max_sin = real_t(0.17);
};


}