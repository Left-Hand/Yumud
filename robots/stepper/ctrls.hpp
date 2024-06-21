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
    Range current_range         {0, 0.7};

    void reset(){
        current_output = 0;
    }


    void setCurrentClamp(const real_t maximum){
        current_range.end = maximum;
    }

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
    virtual void setCurrentClamp(const real_t max_current) = 0;
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

    real_t kp = 3;
    Range kp_clamp = {-20, 20};

    real_t last_speed = 0;
    real_t kd = 70;
    real_t kd_radius = 1;
    Range kd_clamp = {-1, 1};

    real_t targ_current_256x = 0;
    real_t current_clamp_256x = 1.2 * 256;

    real_t last_error = 0;
    real_t lock_radius = 0;
    real_t error;
    real_t delta;
    bool locked = false;

    bool inited = false;

    void reset() override {
        inited = false;
    }

    void setCurrentClamp(const real_t max_current){
        current_clamp_256x = max_current * 256;
    }

    Result update(const real_t targ_speed,const real_t real_speed) override{

        if(!inited){
            targ_current_256x = 0;
            inited = true;
        }

        error = (targ_speed - real_speed);
        // if(ABS(error) > lock_radius) locked = false;
        // bool cross = (error * last_error < 0);
        // last_error = error;

        // if(cross and (not locked)) locked = true;

        // if(not locked){
        real_t speed_delta = real_speed - last_speed;
        last_speed = real_speed; 


        real_t kp_contribute = kp_clamp.clamp(error * kp);
        real_t kd_contribute = (ABS(error) < kd_radius) ? kd_clamp.clamp(kd * speed_delta) : 0;

        // delta = kp_contribute - (kd_contribute << 8);

        targ_current_256x += (kp_contribute >> 8) - kd_contribute; 
        
        if(targ_speed > 0) targ_current_256x = CLAMP(targ_current_256x, 0, current_clamp_256x);
        else targ_current_256x = CLAMP(targ_current_256x, -current_clamp_256x, 0);

        real_t current_out = ABS(targ_current_256x >> 8);
        return {current_out, SIGN_AS((PI / 2 * (1 + MIN(current_out, 0.9))), targ_speed)};
    }
};

struct GeneralPositionCtrl:public PositionCtrl{
    GeneralPositionCtrl(CurrentCtrl & ctrl):PositionCtrl(ctrl){;}

    real_t kp = 9.41;
    Range kp_clamp = {-1.2, 1.2};

    real_t kd = 0.05;
    Range kd_active_range = {0.02, 0.3};
    bool inited = false;

    // real_t kq = 10;
    real_t ki = 0.0;
    real_t intergalx256 = 0;
    Range ki_clamp = {25.6, -25.6};

    real_t last_error;
    real_t last_current;
    real_t error;

    void setCurrentClamp(const real_t max_current) override {

    }

    void reset() override {
        inited = false;
    }

    __fast_inline real_t position2rad(const real_t position){
        real_t frac1 = SIGN_AS(frac(position), position) * poles;
        return (poles * TAU) * (SIGN_AS(frac(abs(frac1)), frac1));
    }

    __fast_inline real_t s(const real_t err, const real_t abs_current){
        static constexpr double u = TAU;
        static constexpr double pu = poles * u;
        static constexpr double a = 2 / PI;
        static constexpr double k = a * a * pu;
        real_t abs_out = PI/2 - 1/(real_t(k) * ABS(err) + a);
        return SIGN_AS(abs_out * (1 + MIN(abs_current, 0.36)) , err);
    }
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad) override{

        if(!inited){
            inited = true;
            intergalx256 = 0;
        }

        error = targ_position - real_position;
        // bool cross_zero = error * last_error < 0;
        // last_error = error;

        real_t raddiff;
            // raddiff = SIGN_AS(PI / 2, error);
        // if(ABS(error) > (inv_poles / 4)){
        //     raddiff = SIGN_AS(PI / 2, error);
        // }else{
        //     raddiff = error * poles * TAU;
        // }

        raddiff = s(error, last_current);

        real_t kp_contribute = kp_clamp.clamp(ABS(error) * kp);

        // if(cross_zero){
        //     intergalx256 = 0;
        // }else{
        //     intergalx256 += ki;
        //     intergalx256 = ki_clamp.clamp(intergalx256);
        // }

        // bool near = (ABS(error) < real_t(0.7));
        // real_t kd_contribute = near ? kd * ABS(real_speed): 0;
        // real_t kd_contribute = kd * ABS(real_speed);

        real_t pid_out = kp_contribute;
        // real_t pid_out =  (intergalx256 >> 8) + kp_contribute - kd_contribute;
        // - kd_contribute;

        // DEBUG_PRINT(ABS(error) < kd_active_radius);
        // real_t kd_contribute = (ABS(error) < kd_active_radius) ? kd * SIGN_AS(real_speed, error) : 0;
        real_t current = MAX(pid_out, 0);
        last_current = current;
        // current *= current * kq;

        return {current, raddiff};
    }
};

struct TopLayerCtrl{

};

struct TrapezoidPosCtrl:public TopLayerCtrl{
    GeneralSpeedCtrl & speed_ctrl;
    GeneralPositionCtrl & position_ctrl;

    TrapezoidPosCtrl(GeneralSpeedCtrl & _speed_ctrl, GeneralPositionCtrl & _position_ctrl):speed_ctrl(_speed_ctrl), position_ctrl(_position_ctrl){;}

    enum Tstatus{
        ACC,
        // SUS,
        DEC,
        STA,
    };
    Tstatus tstatus = Tstatus::STA;
    real_t goal_speed = 0;
        // real_t goal_pos = target;
    real_t last_pos_err = 0;
    using Result = HighLayerCtrl::Result;
    
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad){
        // static constexpr real_t max_acc = 10.0;
        static constexpr real_t max_dec = 168.0;
        static constexpr real_t spd_delta = 0.01;
        static constexpr real_t max_spd = 40;
        static constexpr real_t hug_speed = 0.6;
        // static constexpr real_t spd_sw_radius = 0.7;
        static constexpr real_t pos_sw_radius = 0.1;

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
                // tstatus = Tstatus::STA;

            // case Tstatus::SUS:

                // else if(pos_err * last_pos_err < 0){
                    // tstatus = Tstatus::STA;
                // }
                break;
                // break;
            case Tstatus::DEC:
                // tstatus = Tstatus::STA;
                // break;
                if(cross and ABS(real_speed) < hug_speed){
                    tstatus = Tstatus::STA;
                }

                {
                    bool ovs = real_speed * real_speed > 2 * max_dec* ABS(pos_err);
                    // goal_speed = SIGN_AS(sqrt(2 * max_dec* ABS(pos_err)), goal_speed);
                    if(ovs) goal_speed += SIGN_AS(-spd_delta, pos_err);
                    else goal_speed += SIGN_AS(spd_delta / 10, pos_err);

                    if(pos_err > 0) goal_speed = CLAMP(goal_speed, 0, max_spd);
                    else goal_speed = CLAMP(goal_speed, -max_spd, 0);
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


    bool inited = false;

    OverloadSpeedCtrl(CurrentCtrl & ctrl):SpeedCtrl(ctrl){;}

    void reset() override {
        inited = false;
    }

    void setCurrentClamp(const real_t clamp) override {;}


    Result update(const real_t targ_speed,const real_t real_speed) override{

        if(!inited){
            inited = true;
            elecrad_addition = real_t(0);
        }

        real_t error = SIGN_AS(targ_speed - real_speed, targ_speed);
        // real_t abs_error = abs(error);

        real_t kp_contribute = kp_clamp.clamp(error * kp);
        // intergal = CLAMP(intergal + error, -intergal_clamp, intergal_clamp);
        // real_t ki_contribute = CLAMP(intergal * ki, -ki_clamp, ki_clamp);

        elecrad_addition = CLAMP(elecrad_addition + kp_contribute, real_t(0), elecrad_addition_clamp);

        real_t elecrad_offset = SIGN_AS(real_t(PI / 2) + elecrad_addition, targ_speed);

        return {currCtrl.update(0.37), elecrad_offset};
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

#endif