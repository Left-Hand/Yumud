#include "paraments.hpp"

namespace SMC{
    Measurement measurement;

    Grayscale positive_threshold = 60;
    Grayscale edge_threshold = 30;
    int safety_seed_height = 0;
    real_t dpv(real_t(1.8));

    Rangei valid_width = Rangei(188 / 9, 188 / 4);
    int align_space_width = 12;
    bool align_right = true;
    uint8_t enable_flag = false;
    uint8_t show_status = false;
    int frame_ms = 0;
    TurnCtrl turn_ctrl;
    SideCtrl side_ctrl;
    SpeedCtrl speed_ctrl;
    SideVelocityObserver side_velocity_observer;
    real_t dir_merge_max_sin = real_t(0.17);
    MotorStrength motor_strength;
}