#pragma once

#include "../sys/kernel/clock.h"
#include "../sys/core/system.hpp"

#include "../types/real.hpp"
#include "../types/rect2/rect2_t.hpp"
#include "../types/rgb.h"
#include "body.hpp"


namespace SMC{


struct Measurement{
    real_t dir_error;
    real_t neutral_offset;

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
};


struct TurnCtrl{
    real_t kp = real_t(1);
    real_t kd = real_t(1);

    real_t last_t;
    real_t last_output;
    real_t out_clamp = real_t(0.8);

    void reset(){
        last_t = 0;
        last_output = 0;
    }

    real_t update(const real_t target_dir, const real_t current_dir, const real_t current_omega){
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

struct VelocityCtrl{
    real_t kp = real_t(0.01);
    real_t kd = real_t(0);

    real_t last_output = real_t(0);
    real_t output_clamp = 0.5;

    void reset(){
        last_output = 0;
    }
    real_t update(const real_t targ, const real_t now){

        real_t error = targ - now;
        real_t kp_contribute = kp * error;

        real_t output = last_output;
        
        output += kp_contribute;
        // DEBUG_PRINTL7N(output);
        return last_output = CLAMP(output, 0, output_clamp);
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

    void reset(){
        intergal = 0;
        last_t = 0;
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

    void reset(){
        v1 = 0;
        v2 = 0;
        last_pos = 0;
        last_t = 0;
    }
};

}