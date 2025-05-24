#pragma once

#include "../core/system.hpp"

#include "core/math/real.hpp"
#include "types/regions/rect2/rect2.hpp"
#include "types/rgb.h"


#include "body.hpp"
#include "elements.hpp"


namespace SMC{






struct GlobalConfig{
    Grayscale positive_threshold = 255;
    Grayscale edge_threshold = 2;
    int seed_height_base = 5;
    real_t dpv = 1.8;

    real_t road_width = 0.4;
    Range valid_road_meters = {0.2, 0.6};

    real_t dir_merge_max_sin = real_t(0.27);
};



struct RingConfig{
    real_t s1 = 1.0;
    real_t c1 = 0.3;
    real_t o = 1.6;
    real_t c2 = 0.4;
    real_t s2 = 1.0;
};



union Switches{
    struct{
        bool plot_de:1 = false;
        bool hand_mode:1 = false;
        AlignMode align_mode:2 = AlignMode::BOTH;
        LR element_side:1 = LR::LEFT;
        ElementType element_type:3 = ElementType::NONE;    
        union{
            uint8_t element_status:4;
            Ring::Status ring_status:4;
            Zebra::Status zebra_status:4;
            Barrier::Status barrier_status:4;
            Cross::Status cross_status:4;
        };

    };

    uint16_t data;
};


union Flags{
    struct{
        bool disable_trig:1;
        bool enable_trig:1;
    };

    uint16_t data = 0;
};

struct SetPoints{
    real_t targ_spd = 0.24;
};

struct Benchmark{
    
    struct{
        uint cap;
        uint pers;
        uint gray;
        uint bina;
        uint frame_ms = 0;
    };

};

struct BkpReg{
union{};

};



}