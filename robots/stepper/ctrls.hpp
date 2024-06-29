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
    real_t current_clamp        = 0.7;

    void reset(){
        current_output = 0;
    }

    void setCurrentClamp(const real_t maximum){
        current_clamp = maximum;
    }

    real_t update(const real_t targ_current){
        real_t current_delta = CLAMP(targ_current - current_output, -current_slew_rate, current_slew_rate);
        current_output = MIN(current_output + current_delta, current_clamp);
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
    GeneralSpeedCtrl(CurrentCtrl & ctrl):SpeedCtrl(ctrl){;}

    real_t kp = 4;
    real_t kp_clamp = 60;

    real_t last_speed = 0;
    real_t kd = 40;
    real_t kd_active_radius = 1.2;
    real_t kd_clamp = 2.4;

    real_t targ_current = 0;

    void reset() override {
        targ_current = 0;
    }

    Result update(const real_t targ_speed,const real_t real_speed) override{

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

    real_t kp = 80;
    real_t kd = 50;

    real_t kd_active_radius = 0.7;
    bool inited = false;

    void reset() override {
        inited = false;
    }

    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad) override{

        if(!inited){
            inited = true;
        }

        real_t error = targ_position - real_position;
        real_t abs_err = ABS(error);


        static constexpr double u = TAU;
        static constexpr double pu = poles * u;
        static constexpr double a = 2 / PI;
        static constexpr double k = a * a * pu;

        real_t abs_uni_raddiff = real_t(PI/2) - 1/(real_t(k) * abs_err + a);
        // real_t abs_uni_raddiff = MIN(abs_err * pu, PI/2);
        real_t raddiff = abs_uni_raddiff * SIGN_AS((1 + MIN(curr_ctrl.current_output, 0.42)) , error);


        real_t current = MIN(abs_err * kp, curr_ctrl.current_clamp); 
        if(abs_err < kd_active_radius) current = MAX(current - (kd * ABS(real_speed) >> 8), 0);
        
        if(error * real_speed < 0){
            return {current, SIGN_AS(PI / 2, error)};
        }else{
            return {current, raddiff};
        }
    }
};
struct SpeedEstimator{
public:
    real_t last_position = 0;
    // real_t delta_speed_per_cycle = 0;
    real_t last_speed = 0;
    size_t cycles = 1;
    static constexpr real_t err_threshold = inv_poles/4;
    static constexpr size_t max_cycles = foc_freq / 40;

    void reset(){
        *this = SpeedEstimator();
    }

    real_t update(const real_t position){
        real_t delta_pos = position - last_position;
        real_t abs_delta_pos = ABS(delta_pos);
        real_t this_speed = (delta_pos * int(foc_freq) / int(cycles));

        if(abs_delta_pos > err_threshold){//high speed one cycle
            // real_t delta_speed_per_cycle_clamp = ABS(last_speed / (cycles * 8));
            // delta_speed_per_cycle = SIGN_AS(MIN(ABS(this_speed - last_speed)/cycles, delta_speed_per_cycle_clamp), this_speed - last_speed);

            cycles = 1;
            last_position = position;
            return last_speed = this_speed;
        }else{
            cycles++;
            if(cycles > max_cycles){
                
                cycles = 1;
                last_position = position;
                // delta_speed_per_cycle = 0;
                return last_speed = this_speed;
            }
        }

        // if(ABS(last_speed) < (1)) return last_speed + delta_speed_per_cycle * cycles;
        return last_speed;
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
    real_t last_pos_err = 0;
    using Result = HighLayerCtrl::Result;
    
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad){
        // static constexpr real_t max_acc = 10.0;
        static constexpr real_t max_dec = 168.0;
        static constexpr real_t spd_delta = max_dec/foc_freq;
        // static constexpr real_t spd_delta = 0.01;
        static constexpr real_t max_spd = 40;
        static constexpr real_t hug_speed = 0.1;
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
                break;

            case Tstatus::DEC:
                if(cross and (ABS(goal_speed) < hug_speed)){
                    tstatus = Tstatus::STA;
                }

                {
                    bool ovs = real_speed * real_speed > 2 * max_dec* ABS(pos_err);
                    // goal_speed = SIGN_AS(sqrt(2 * max_dec* ABS(pos_err)), goal_speed);
                    if(ovs) goal_speed += SIGN_AS(-spd_delta, pos_err);
                    else goal_speed += SIGN_AS(spd_delta / 3, pos_err);

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

        return {curr_ctrl.update(0.37), elecrad_offset};
    }
};


#endif