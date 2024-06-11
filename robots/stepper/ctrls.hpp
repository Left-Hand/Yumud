#ifndef __STEPPER_CTRLS_HPP__

#define __STEPPER_CTRLS_HPP__

#include "constants.hpp"


struct LowerLayerCtrl{
    virtual void reset() = 0;
    virtual real_t update(const real_t target) = 0;
};

struct CurrentCtrl{
protected:
    using Range = Range_t<real_t>;
public:

    real_t current_slew_rate    = 20.0 / foc_freq;   //20A/S
    real_t current_output       = 0;
    Range current_range         {0, 0.2};

    void reset(){
        current_output = 0;
    }

    void setRange(const real_t maximum){current_range.end = maximum;}
    real_t update(const real_t target){
        real_t current_delta = CLAMP(target - current_output, -current_slew_rate, current_slew_rate);
        current_output = current_range.clamp(current_output + current_delta);
        return current_output;
    }
};



struct HighLayerCtrl{
public:

    struct Result{
        real_t current;
        real_t raddiff;
    };
protected:
    using Range = Range_t<real_t>;
    CurrentCtrl & currCtrl;

public:

    HighLayerCtrl(CurrentCtrl & _ctrl):currCtrl(_ctrl){;}

    virtual void reset() = 0;
};

struct SpeedCtrl:public HighLayerCtrl{
    SpeedCtrl(CurrentCtrl & ctrl):HighLayerCtrl(ctrl){;}
    virtual Result update(const real_t targ_speed,const real_t real_speed) = 0;
};

struct PositionCtrl:public HighLayerCtrl{
    PositionCtrl(CurrentCtrl & ctrl):HighLayerCtrl(ctrl){;}
    virtual Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad) = 0;
};

struct GeneralSpeedCtrl:public SpeedCtrl{
    GeneralSpeedCtrl(CurrentCtrl & ctrl):SpeedCtrl(ctrl){;}

    real_t kp = 0.11;
    Range kp_clamp = {-0.7, 0.7};

    real_t last_speed = 0;
    real_t kd = 10;

    real_t targ_current_256x = 0;
    Range current_clamp_256x = {0, 51.6};

    bool inited = false;

    void reset() override {
        inited = false;
    }

    Result update(const real_t targ_speed,const real_t real_speed) override{

        if(!inited){
            targ_current_256x = 0;
            inited = true;
        }

        real_t error = SIGN_AS((targ_speed - real_speed), targ_speed);

        real_t abs_real_speed = ABS(real_speed);
        real_t speed_delta = abs_real_speed - last_speed;
        last_speed = abs_real_speed; 

        real_t kd_scaler = 1;
        if(abs_real_speed < 1) kd_scaler = (abs_real_speed * abs_real_speed);

        real_t kp_contribute = kp_clamp.clamp(error * kp);
        real_t kd_contribute = kd * speed_delta * kd_scaler;

        real_t delta = kp_contribute - kd_contribute;

        targ_current_256x = current_clamp_256x.clamp(targ_current_256x + delta);

        return {targ_current_256x >> 8, SIGN_AS(PI / 2, targ_speed)};
    }
};

struct GeneralPositionCtrl:public PositionCtrl{
    GeneralPositionCtrl(CurrentCtrl & ctrl):PositionCtrl(ctrl){;}

    real_t kp = 4.47;
    Range kp_clamp = {-0.2, 0.2};

    real_t min_current = 0.02;
    real_t kd = 0.01;

    Range kd_active_range = {0.02, 0.1};
    bool inited = false;

    void reset() override {
        inited = false;
    }

    __fast_inline real_t position2rad(const real_t position){
        real_t abs_frac1 = poles * abs(position);
        return SIGN_AS(TAU * (frac(abs_frac1)), position);
    }


    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad) override{

        if(!inited){
            inited = true;
        }

        real_t error = targ_position - real_position;
        real_t raddiff = (error *50 *TAU);
        if(ABS(raddiff) > PI / 2 ) raddiff = SIGN_AS(PI / 2, raddiff);
        // else raddiff = raddiff - real_elecrad;

        real_t kp_contribute = kp_clamp.clamp(ABS(error) * kp);
        bool near = kd_active_range.has(ABS(error));
        real_t kd_contribute = near ? kd * ABS(real_speed): 0;

        real_t pid_out = kp_contribute - kd_contribute;

        // DEBUG_PRINT(ABS(error) < kd_active_radius);
        // real_t kd_contribute = (ABS(error) < kd_active_radius) ? kd * SIGN_AS(real_speed, error) : 0;
        real_t current = MAX(pid_out, 0);
        return {current, raddiff};
    }
};

// struct OverSpeedCtrl:public SpeedCtrl{

//     real_t kp = real_t(0.03);
//     Range kp_clamp = {-0.1, 0.1};

//     real_t ki;
//     real_t intergal;
//     real_t intergal_clamp;
//     Range ki_clamp;

//     real_t elecrad_addition;
//     real_t elecrad_addition_clamp = real_t(0.8);


//     bool inited = false;

//     OverSpeedCtrl(CurrentCtrl & ctrl):SpeedCtrl(ctrl){;}

//     void reset() override {
//         inited = false;
//     }


//     Result update(const real_t targ_speed,const real_t real_speed) override{

//         if(!inited){
//             inited = true;
//             elecrad_addition = real_t(0);
//         }

//         real_t error = targ_speed - real_speed;
//         if(targ_speed < 0) error = - error;
//         // real_t abs_error = abs(error);

//         real_t kp_contribute = kp_clamp.clamp(kp);
//         intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
//         real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

//         elecrad_addition = CLAMP(elecrad_addition + kp_contribute + ki_contribute, real_t(0), elecrad_addition_clamp);

//         real_t elecrad_offset = SIGN_AS(real_t(PI / 2) + elecrad_addition, targ_speed);

//         return {currCtrl.update(0.2), elecrad_offset};
//     }
// // };

// struct PositionCtrl{
//     real_t kp;
//     real_t ki;
//     real_t kd;
//     real_t ks;

//     real_t kp_clamp;
//     real_t intergal;
//     real_t ki_clamp;
//     real_t kd_enable_speed_threshold;//minimal speed for activation kd
//     real_t kd_clamp;
//     real_t ks_enable_speed_threshold; // minimal speed for activation ks
//     real_t ks_clamp;

//     real_t target_position;
//     real_t target_speed;

//     real_t current_slew_rate = real_t(20.0 / foc_freq);
//     real_t current_minimal = real_t(0.05);
//     real_t current_clamp = real_t(0.3);

//     real_t err_to_current_ratio = real_t(20);

//     real_t current_output;
//     real_t elecrad_output;
//     real_t last_error;

//     auto update(const real_t & _target_position, const real_t & measured_position, const real_t & measuresd_speed){
//         target_position = _target_position;
//         real_t error = target_position - measured_position;

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

//         if(true){
//             real_t abs_error = abs(error);
//             current_output = CLAMP(abs_error * err_to_current_ratio, current_minimal, current_clamp);
//             // if(abs(error) < inv_poles * 2){
//             if(false){
//                 // elecrad_offset_output = error * poles * TAU;
//                 // elecrad_output = odo.position2rad(target_position);
//                 //TODO
//                 // elecrad_output = real_t(0);
//             }else{
//                 // elecrad_output = est_elecrad + (error > 0 ? 2.5 : - 2.5);
//                 //TODO
//                 // elecrad_output = real_t(0);
//             }
//         }

//         return std::make_tuple(current_output, elecrad_output);
//     }
// };

#endif