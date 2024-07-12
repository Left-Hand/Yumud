#ifndef __PLL_HPP__

#define __PLL_HPP__

#include "../types/real.hpp"

struct Pll{
    real_t kp;
    real_t ki;

    real_t kp_clamp;
    real_t intergal;
    real_t ki_clamp;

    real_t omega;
    real_t lap_rad;
    real_t last_lap_rad;
    real_t rad;

    real_t filted_rad_delta;

    real_t min_input;
    real_t max_input;

    Pll(){
        kp = real_t(0.1);
        ki = real_t(0);
        kp_clamp = real_t(100);
        ki_clamp = real_t(10);

        min_input = real_t(-1);
        max_input = real_t(1);
    }

    bool inited = false;
    real_t update(const real_t & input, const real_t & delta_t){
        if(inited == false){
            intergal = real_t(0);
            omega = real_t(0);
            lap_rad = real_t(0);
            rad = real_t(0);
            inited = true;
            return real_t();
        }

        real_t uni_input = INVLERP(input, min_input, max_input) * 2 - 1;

        if(is_equal_approx(uni_input, real_t(1))){
            lap_rad = real_t(PI / 2);
        }else if(is_equal_approx(uni_input, real_t(-1))){
            lap_rad = real_t(-PI / 2);
        }else{
            lap_rad = asin(uni_input);
        }


        real_t rad_delta = lap_rad - last_lap_rad;
        last_lap_rad = lap_rad;

        real_t filted_rad_delta = abs(rad_delta) * 0.1 + filted_rad_delta * 0.9;

        // static PID_t<real_t>phase_pid(1.0, 0, 0, PI / 2);
        // static real_t phase = real_t(0);
        // phase = phase_pid.update(rad_delta, filted_rad_delta);
        rad += filted_rad_delta;

        if(rad > TAU) rad -= TAU;

        last_lap_rad = lap_rad;

        return rad;
    }
};

#endif