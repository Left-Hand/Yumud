#pragma once

#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"
#include "body.hpp"

namespace SMC{



#pragma pack(push, 1)

struct Measurement{
    real_t dir_error;

    Quat accel_offs;
    Vector3 gyro_offs;
    Quat magent_offs;

    Vector3 accel;
    Vector3 gyro;
    Vector3 magent;

    real_t current_dir;

    Vector2i seed_pos;
    Rangei road_window;

    real_t front_spd;
    real_t omega;

    real_t side_offs_err;
};


struct GlobalConfig{
    Grayscale positive_threshold = 255;
    Grayscale edge_threshold = 2;
    int safety_seed_height = 5;
    real_t dpv = 1.8;

    Rangei valid_width = Rangei(188 / 9, 188 / 4);
    int align_space_width = 15;

    real_t dir_merge_max_sin = real_t(0.17);
};


enum class AlignMode{
    BOTH,
    LEFT,
    RIGHT,
};

union Switches{
    struct{
        bool plot_de:1;
        bool hand_mode:1;
        LR align_right:1 = LR::RIGHT;
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
    real_t targ_spd;
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