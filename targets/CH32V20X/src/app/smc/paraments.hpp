#ifndef __PARAMENTS_HPP__

#define __PARAMENTS_HPP__



#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"
#include "body.hpp"

using namespace Sys;


namespace SMC{
    struct Measurement{
        real_t dir_error;
        real_t neutral_offset;
    };

    struct TurnCtrl{
        real_t kp = real_t(1);
        real_t ki = real_t(0);
        real_t kd = real_t(1);

        real_t intergal = real_t(0);
        real_t intergal_clamp = real_t(0.1);

        real_t last_t;
        real_t last_output;
        real_t out_clamp = real_t(0.8);

        real_t update(const real_t & target_dir, const real_t & current_dir, const real_t & current_omega){
            real_t error_dir = target_dir - current_dir;
            real_t kp_contribute = kp * error_dir;

            real_t kd_contribute = kd * current_omega;
            real_t output = (kp_contribute + kd_contribute);
            output = CLAMP(output, -out_clamp, out_clamp);
            last_t = t;
            last_output = output;
            return output;
        }
    };

    struct SpeedCtrl{
        real_t update(){
            return real_t(0.45);
        }
    };

    struct SideCtrl{
        real_t kp = real_t(6.0);
        real_t ki = real_t(0.0);
        real_t kd = real_t(0.7);

        real_t intergal = real_t(0);
        real_t intergal_clamp = real_t(0.15);

        real_t last_t;
        real_t update(const real_t & target_offs, const real_t & current_offs, const real_t & measured_velocity){
            real_t error_dir = target_offs - current_offs;
            real_t kp_contribute = kp * error_dir;

            intergal = CLAMP(intergal + (t - last_t) * ki, -intergal_clamp, intergal_clamp);
            real_t ki_contribute = ki * intergal;

            static constexpr real_t eps_dir = real_t(0.01);
            if(abs(error_dir) < eps_dir){
                intergal /= 2;
            }

            real_t kd_contribute = kd * measured_velocity;
            real_t output = kp_contribute + ki_contribute + kd_contribute;
            last_t = t;
            return output;
        }
    };

    struct SideVelocityObserver{
    protected:
        real_t v1 = 0;
        real_t v2 = 0;
    public:
        real_t last_pos;
        real_t last_t = 0;
        real_t k1 = 0.4;
        real_t k2 = 0.4;

        real_t out_clamp = real_t(1);
        real_t update(const real_t & pos, const real_t & acc){
            real_t delta = (t - last_t);
            v1 += acc * delta;
            v1 = k1 * v1 + (1-k1) * (pos - last_pos) / delta;
            v2 = CLAMP(v2 * k2 + (1-k2) * v1, -out_clamp, out_clamp);
            last_pos = pos;
            last_t = t;
            return v2;
        }
    };


    extern TurnCtrl turn_ctrl;
    extern SideCtrl side_ctrl;
    extern SpeedCtrl speed_ctrl;
    extern SideVelocityObserver side_velocity_observer;
    extern Grayscale positive_threshold;
    extern Grayscale edge_threshold ;
    extern int safety_seed_height ;
    extern real_t dpv;
    extern Rangei valid_width ;
    extern int align_space_width ;
    extern bool align_right ;
    extern uint8_t show_status;
    extern int frame_ms;
    extern real_t dir_merge_max_sin;
    extern MotorStrength motor_strength;
    extern uint8_t enable_flag;
};

#endif