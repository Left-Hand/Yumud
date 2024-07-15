#pragma once

#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"
#include "body.hpp"

namespace SMC{



#pragma pack(push, 1)


struct GlobalConfig{
    Grayscale positive_threshold = 60;
    Grayscale edge_threshold = 30;
    int safety_seed_height = 0;
    real_t dpv = 1.8;

    Rangei valid_width = Rangei(188 / 9, 188 / 4);
    int align_space_width = 12;

    real_t dir_merge_max_sin = real_t(0.17);
};

union Flags{
    struct{
        bool disable_trig:1;
        bool enable_trig:1;
        bool plot_de:1;
        bool align_right:1;
        bool hand_mode:1;
    };

    uint16_t data = 0;
};

struct Benchmark{
    
    struct{
        uint cap;
        uint pers;
        uint gray;
        uint bina;
        uint frame_ms = 0;
    };

    // uint traps[];
};

struct BkpReg{
union{};

};

#pragma pack(pop)

}