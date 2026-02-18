#pragma once

#include "core/system.hpp"

#include "core/math/real.hpp"
#include "algebra/regions/rect2.hpp"
#include "algebra/rgb.h"
#include "body.hpp"


namespace SMC{


struct TurnCtrl{
    iq16 kp = iq16(0.6);
    iq16 kd = iq16(1);

    iq16 last_t;
    iq16 last_output;
    iq16 out_clamp = iq16(0.8);

    void reset(){
        last_t = 0;
        last_output = 0;
    }

    iq16 update(const iq16 target_dir, const iq16 current_dir, const iq16 current_omega){
        iq16 error_dir = target_dir - current_dir;
        iq16 kp_contribute = kp * error_dir;

        iq16 kd_contribute = kd * current_omega;
        iq16 output = (kp_contribute + kd_contribute);
        output = CLAMP(output, -out_clamp, out_clamp);
        last_t = t;
        last_output = output;
        return output;
    }
};

struct VelocityCtrl{
    iq16 kp = iq16(0.05);
    iq16 kd = iq16(0);

    iq16 last_output = iq16(0);
    iq16 output_clamp = 0.5;

    void reset(){
        last_output = 0;
    }
    iq16 update(const iq16 targ, const iq16 now){

        iq16 error = targ - now;
        iq16 kp_contribute = kp * error;

        iq16 output = last_output;
        
        output += kp_contribute;
        // DEBUG_PRINTL7N(output);
        // return last_output = CLAMP(output, 0, output_clamp);
        return targ;
    }
};


struct SideCtrl{
    iq16 kp = iq16(5);
    iq16 ki = iq16(0.0);
    iq16 kd = iq16(1.2);

    iq16 intergal = iq16(0);
    iq16 intergal_clamp = iq16(0.15);

    iq16 last_t;
    iq16 update(const iq16 target_offs, const iq16 current_offs, const iq16 measured_velocity){
        iq16 error_dir = target_offs - current_offs;
        iq16 kp_contribute = kp * error_dir;

        intergal = CLAMP(intergal + (t - last_t) * ki, -intergal_clamp, intergal_clamp);
        iq16 ki_contribute = ki * intergal;

        static constexpr iq16 eps_dir = iq16(0.01);
        if(abs(error_dir) < eps_dir){
            intergal /= 2;
        }

        iq16 kd_contribute = kd * measured_velocity;
        iq16 output = kp_contribute + ki_contribute + kd_contribute;
        last_t = t;
        return output;
    }
               
    void reset(){
        intergal = 0;
        last_t = 0;
    }
};

struct CentripetalCtrl{
    iq16 k = iq16(2.0);                    
    iq16 k_clamp = iq16(0.84);                        
                 
    iq16 update(const iq16 spd, const iq16 omega){               
        return CLAMP(k * spd * omega, -k_clamp, k_clamp);
    };
};


struct SideVelocityObserver{
protected:
    iq16 v1 = 0;
    iq16 v2 = 0;
public:
    iq16 last_pos;
    iq16 last_t = 0;
    iq16 k1 = 0.4;
    iq16 k2 = 0.4;

    iq16 out_clamp = iq16(1);
    iq16 update(const iq16 pos, const iq16 acc){
        iq16 delta = (t - last_t);
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