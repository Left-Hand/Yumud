#pragma once

#include "constants.hpp"
#include "configs.hpp"

#ifdef DEBUG
#define CURR_SPEC protect
#define POS_SPEC protect
#define SPD_SPEC protect
#else
#define CURR_SPEC public
#define POS_SPEC public
#define SPD_SPEC public
#endif

struct CtrlResult{
    real_t current;
    real_t raddiff;
};

using Result = CtrlResult;
struct CurrentCtrl{
CURR_SPEC:
    real_t current_output = 0;
public:
    CurrentCtrlConfig & config;

    real_t & current_slew_rate = config.current_slew_rate;
    real_t & current_clamp = config.current_clamp;


    CurrentCtrl(CurrentCtrlConfig & _config):config(_config){
        config.init();
    }

    void reset(){
        current_output = 0;
    }

    void setCurrentClamp(const real_t clp){
        current_clamp = clp;
    }

    real_t update(const real_t targ_current){
        real_t current_delta = CLAMP(targ_current - current_output, -current_slew_rate, current_slew_rate);
        current_output = MIN(current_output + current_delta, current_clamp);
        return current_output;
    }
};



struct HighLayerCtrl{
protected:
    CurrentCtrl & curr_ctrl;
public:

    HighLayerCtrl(CurrentCtrl & _ctrl):curr_ctrl(_ctrl){;}
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
public:
    GeneralSpeedCtrlConfig &config;
SPD_SPEC:
    const real_t & max_spd = config.max_spd;
    const real_t & kp = config.kp;
    const real_t & kp_clamp = config.kd_clamp;

    const real_t & kd = config.kd;
    const real_t & kd_active_radius = config.kd_active_radius;
    const real_t & kd_clamp = config.kd_clamp;

    real_t targ_current = 0;
    real_t last_speed = 0;

public:
    GeneralSpeedCtrl(CurrentCtrl & ctrl, GeneralSpeedCtrlConfig & _config):SpeedCtrl(ctrl), config(_config){;}
    void reset() override {
        targ_current = 0;
    }

    Result update(const real_t _targ_speed,const real_t real_speed) override{
        const real_t targ_speed = CLAMP(_targ_speed, -max_spd, max_spd);
        real_t error = (targ_speed - real_speed);

        real_t speed_delta = real_speed - last_speed;
        last_speed = real_speed; 

        real_t kp_contribute = CLAMP(error * kp, -kp_clamp, kp_clamp);
        real_t kd_contribute = CLAMP(kd * speed_delta, -kd_clamp, kd_clamp);

        if((ABS(error) > kd_active_radius)) kd_contribute /= 3;

        targ_current += (kp_contribute >> 16) - (kd_contribute >> 8); 

        real_t abs_targ_current = MIN(ABS(targ_current), curr_ctrl.current_clamp);
        targ_current = SIGN_AS(abs_targ_current, targ_speed);

        if(real_speed * targ_speed > 0) return {abs_targ_current, SIGN_AS((PI / 2 * (1 + MIN(curr_ctrl.current_output, 1.2))), targ_speed)};
        else return {abs_targ_current, SIGN_AS(PI / 2, targ_speed)};
    }
};

struct GeneralPositionCtrl:public PositionCtrl{
    GeneralPositionCtrl(CurrentCtrl & ctrl):PositionCtrl(ctrl){;}

    real_t kp = 60;
    real_t kd = 5;

    real_t kd_active_radius = 0.7;

    real_t ki = 0.0;
    real_t intergalx256 = 0;
    Range ki_clamp = {25.6, -25.6};

    real_t last_error;
    real_t last_current;
    real_t error;


    void reset() override {
        intergalx256 = 0;
    }

    Result update(const real_t targ_position, const real_t real_position, 
            const real_t real_speed, const real_t real_elecrad) override{

        real_t error = targ_position - real_position;
        real_t abs_err = ABS(error);


        static constexpr double u = TAU;
        static constexpr double pu = poles * u;
        static constexpr double a = 2 / PI;
        static constexpr double k = a * a * pu;

        real_t abs_uni_raddiff = real_t(PI/2) - 1/(real_t(k) * abs_err + a);

        real_t raddiff = abs_uni_raddiff * SIGN_AS((1 + MIN(curr_ctrl.current_output, 0.45)) , error);

        real_t current = MIN(abs_err * kp, curr_ctrl.current_clamp); 
        if(abs_err < kd_active_radius) current = MAX(current - (kd * ABS(real_speed) >> 8), 0);
        
        if(error * real_speed < 0){
            return {current, SIGN_AS(PI / 2, error)};
        }else{
            return {current, raddiff};
        }
    }
};

struct TopLayerCtrl{

};

struct TrapezoidPosCtrl:public TopLayerCtrl{
    GeneralSpeedCtrl & speed_ctrl;
    GeneralPositionCtrl & position_ctrl;

    real_t max_dec = 18.0;

    TrapezoidPosCtrl(GeneralSpeedCtrl & _speed_ctrl, GeneralPositionCtrl & _position_ctrl):speed_ctrl(_speed_ctrl), position_ctrl(_position_ctrl){;}

    enum Tstatus{
        ACC,
        // SUS,
        DEC,
        STA,
    };

protected:
    Tstatus tstatus = Tstatus::STA;
    real_t goal_speed = 0;
    real_t last_pos_err = 0;
public:
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad){
        // static constexpr real_t max_acc = 10.0;

        real_t spd_delta = max_dec/foc_freq;
        real_t max_spd = speed_ctrl.max_spd;
        // static constexpr real_t spd_delta = 0.01;

        // static constexpr real_t hug_speed = 1;
        // static constexpr real_t spd_sw_radius = 0.7;
        static constexpr real_t pos_sw_radius = 0.6;

        real_t pos_err = targ_position - real_position;
        bool cross = pos_err * last_pos_err < 0;
        last_pos_err = pos_err;
        switch(tstatus){
            case Tstatus::ACC:
                if(real_speed * real_speed > 2 * max_dec* ABS(pos_err)){
                    tstatus = Tstatus::DEC;
                }

                {
                    goal_speed += SIGN_AS(spd_delta, pos_err);
                    goal_speed = CLAMP(goal_speed, -max_spd, max_spd);
                    return speed_ctrl.update(goal_speed, real_speed);
                }
                break;

            case Tstatus::DEC:
                // if((cross and (ABS(goal_speed) < hug_speed)) || abs(pos_err) < pos_sw_radius){
                //     tstatus = Tstatus::STA;
                // }
                if(cross){
                    tstatus = Tstatus::STA;
                }

                {
                    // bool ovs = real_speed * real_speed > 2 * max_dec* ABS(pos_err);
                    // goal_speed = SIGN_AS(sqrt(2 * max_dec* ABS(pos_err)), goal_speed);
                    // if(ovs) goal_speed += SIGN_AS(-spd_delta, pos_err);
                    // if(ovs) goal_speed += -spd_delta;
                    // else goal_speed += SIGN_AS(spd_delta / 3, pos_err);

                    // if(pos_err > 0) goal_speed = CLAMP(goal_speed, hug_speed, max_spd);
                    // else goal_speed = CLAMP(goal_speed, -max_spd, -hug_speed);

                    goal_speed = SIGN_AS(sqrt(2 * max_dec * ABS(pos_err)), pos_err);
                    return speed_ctrl.update(goal_speed, real_speed);
                }

                break;
            default:
            case Tstatus::STA:
                if(ABS(pos_err) > pos_sw_radius){
                    goal_speed = 0;
                    tstatus = Tstatus::ACC;
                }
                return position_ctrl.update(targ_position, real_position, real_speed, real_elecrad);

                break;
        }
        
    }
};


struct OverloadSpeedCtrl:public SpeedCtrl{

    real_t kp = real_t(0.00014);
    Range kp_clamp = {-0.1, 0.1};



    real_t elecrad_addition;
    real_t elecrad_addition_clamp = real_t(1.7);

    OverloadSpeedCtrl(CurrentCtrl & ctrl):SpeedCtrl(ctrl){;}

    void reset() override {
        elecrad_addition = real_t(0);
    }


    Result update(const real_t targ_speed,const real_t real_speed) override{

        real_t error = SIGN_AS(targ_speed - real_speed, targ_speed);
        // real_t abs_error = abs(error);

        real_t kp_contribute = kp_clamp.clamp(error * kp);
        // intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
        // real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

        elecrad_addition = CLAMP(elecrad_addition + kp_contribute, real_t(0), elecrad_addition_clamp);

        real_t elecrad_offset = SIGN_AS(real_t(PI / 2) + elecrad_addition, targ_speed);

        return {curr_ctrl.update(0.37), elecrad_offset};
    }
};

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
