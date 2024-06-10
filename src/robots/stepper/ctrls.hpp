#ifndef __STEPPER_CTRLS_HPP__

#define __STEPPER_CTRLS_HPP__

#include "constants.hpp"

struct CurrentCtrl{
    using Range = Range_t<real_t>;
public:
    real_t current_slew_rate = real_t(20.0 / foc_freq);
    real_t current_output = real_t(0);
    Range current_range{real_t(-0.4), real_t(0.4)};

    real_t update(const real_t & current_setpoint){
        real_t current_delta = CLAMP(current_setpoint - current_output, -current_slew_rate, current_slew_rate);
        current_output = current_range.clamp(current_output + current_delta);
        return current_output;
    }
};

struct TorqueCtrl{
    CurrentCtrl & currCtrl;

};

struct SpeedCtrl{
    real_t kp = real_t(0.03);
    real_t ki;

    real_t kp_clamp = real_t(0.1);
    real_t intergal;
    real_t intergal_clamp;
    real_t ki_clamp;



    real_t current_output;
    real_t elecrad_offset_output;

    real_t elecrad_addition;
    real_t elecrad_addition_clamp = real_t(0.8);


    bool inited = false;
    auto update(const real_t & goal_speed,const real_t & measured_speed){
        if(!inited){
            inited = true;
            elecrad_offset_output = real_t(0);
            elecrad_addition = real_t(0);
        }

        real_t error = goal_speed - measured_speed;
        if(goal_speed < 0) error = - error;
        // real_t abs_error = abs(error);

        real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
        intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
        real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

        elecrad_addition = CLAMP(elecrad_addition + kp_contribute + ki_contribute, real_t(0), elecrad_addition_clamp);

        real_t elecrad_offset = real_t(PI / 2) + elecrad_addition;
        if(goal_speed < 0) elecrad_offset = - elecrad_offset;

        // current_output = targets.curr;
        // elecrad_offset_output = elecrad_offset;

        // return std::make_tuple(current_output, elecrad_offset + odo.getElecRad());
        //TODO
        return std::make_tuple(current_output, elecrad_offset);
    }

    real_t getElecradOffset(){
        return elecrad_offset_output;
    }

    real_t getCurrent(){
        return current_output;
    }
};

struct PositionCtrl{
    real_t kp;
    real_t ki;
    real_t kd;
    real_t ks;

    real_t kp_clamp;
    real_t intergal;
    real_t ki_clamp;
    real_t kd_enable_speed_threshold;//minimal speed for activation kd
    real_t kd_clamp;
    real_t ks_enable_speed_threshold; // minimal speed for activation ks
    real_t ks_clamp;

    real_t target_position;
    real_t target_speed;

    real_t current_slew_rate = real_t(20.0 / foc_freq);
    real_t current_minimal = real_t(0.05);
    real_t current_clamp = real_t(0.3);

    real_t err_to_current_ratio = real_t(20);

    real_t current_output;
    real_t elecrad_output;
    real_t last_error;

    auto update(const real_t & _target_position, const real_t & measured_position, const real_t & measuresd_speed){
        target_position = _target_position;
        real_t error = target_position - measured_position;

        // real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);

        // if((error.value ^ last_error.value) & 0x8000){ // clear intergal when reach target
        //     intergal = real_t(0);0
        // }else{
        //     intergal += error;
        // }

        // real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

        // real_t kd_contribute;
        // if(abs(measuresd_speed) > kd_enable_speed_threshold){ // enable kd only highspeed
        //     kd_contribute =  CLAMP(- measuresd_speed * kd, -kd_clamp, kd_clamp);
        // }else{
        //     kd_contribute = real_t(0);
        // }

        // real_t speed_error = target_speed - measuresd_speed;
        // real_t ks_contribute;
        // if(abs(speed_error) > ks_enable_speed_threshold){
        //     ks_contribute = CLAMP(speed_error * ks, -ks_clamp, ks_clamp);
        // }else{
        //     ks_contribute = real_t(0);
        // }

        // last_error = error;

        // real_t current_delta = CLAMP(kp_contribute + ki_contribute, -current_slew_rate, current_slew_rate);
        // current_output = CLAMP(current_output + current_delta, -current_clamp, current_clamp);
        // // return current_slew_rate;

        if(true){
            real_t abs_error = abs(error);
            current_output = CLAMP(abs_error * err_to_current_ratio, current_minimal, current_clamp);
            // if(abs(error) < inv_poles * 2){
            if(false){
                // elecrad_offset_output = error * poles * TAU;
                // elecrad_output = odo.position2rad(target_position);
                //TODO
                // elecrad_output = real_t(0);
            }else{
                // elecrad_output = est_elecrad + (error > 0 ? 2.5 : - 2.5);
                //TODO
                // elecrad_output = real_t(0);
            }
        }

        return std::make_tuple(current_output, elecrad_output);
    }
};

#endif