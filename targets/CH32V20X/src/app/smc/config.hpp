#pragma once

#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"


#include "body.hpp"
#include "elements.hpp"


namespace SMC{



#pragma pack(push, 1)



struct Measurement{

    real_t front_spd;
    real_t lane_offset;

    Vector2i seed_pos;
    real_t dir_error;

    real_t current_dir;
    Rangei road_window;
};


struct GlobalConfig{
    Grayscale positive_threshold = 255;
    Grayscale edge_threshold = 2;
    int seed_height_base = 5;
    real_t dpv = 1.8;

    real_t road_width = 0.4;
    Range valid_road_meters = {0.2, 0.6};

    real_t dir_merge_max_sin = real_t(0.27);
};



union Switches{
    struct{
        bool plot_de:1 = false;
        bool hand_mode:1 = false;
        AlignMode align_mode:2 = AlignMode::BOTH;
        ElementType element_type:3 = ElementType::ZEBRA;
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
    real_t targ_spd = 0.26;
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

#pragma pack(pop)

}