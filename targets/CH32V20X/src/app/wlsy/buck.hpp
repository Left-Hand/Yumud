#ifndef __BUCK_HPP__

#define __BUCK_HPP__

#include "wlsy_inc.hpp"

namespace WLSY{

using Sys::t;
using Range = Range_t<real_t>;

class Buck{
public:
    real_t output;
protected:
    INA226 & ina;
    PwmChannel & pwm;

    real_t target_watt;

    Range  pwm_duty_range{real_t(0.02), real_t(0.9)};

    struct CurrentController{
    public:
        real_t kp = real_t(0.057);

        real_t & output;
        Range  & pwm_duty_range;

        CurrentController(real_t & _output, Range & _output_range):output(_output), pwm_duty_range(_output_range){;}
        void update(const real_t targ_current, const real_t real_current){
            auto error = targ_current - real_current;
            real_t kp_contribute = error * kp;

            real_t delta = kp_contribute;

            output += delta;
            output = pwm_duty_range.clamp(output);
        }
    };

    class PowerController{
    protected:
        real_t targ_current = 0;
        Range range_current{0, 4};
    public:
        real_t kp = real_t(0.00282);

        CurrentController & curr_ctrl;

        PowerController(CurrentController & _curr_ctrl):curr_ctrl(_curr_ctrl){;}
        void update(const real_t targ_power, const real_t real_power, const real_t real_current){
            auto error = targ_power - real_power;
            real_t kp_contribute = error * kp;

            real_t delta = kp_contribute;

            targ_current += delta;
        targ_current = range_current.clamp(targ_current);

            curr_ctrl.update(targ_current, real_current);
        }
    };

    // class VoltageController{
    //     protected:
    //         real_t targ_current = 0;
    //     public:
    //         real_t kp = real_t(0.00032);

    //         CurrentController & curr_ctrl;

    //         PowerController(CurrentController & _curr_ctrl):curr_ctrl(_curr_ctrl){;}
    //         void update(const real_t targ_voltage, const real_t real_voltage, const real_t real_current){
    //             auto error = targ_voltage - real_voltage;
    //             real_t kp_contribute = error * kp;

    //             real_t delta = kp_contribute;

    //             targ_current += delta;
    //             targ_current = range_current.clamp(targ_current);

    //             curr_ctrl.update(targ_current, real_current);
    //         }
    // };

    CurrentController curr_ctrl{output, pwm_duty_range};
    PowerController power_ctrl{curr_ctrl};

public:
    Buck(INA226 & _ina, PwmChannel & _pwm):ina(_ina), pwm(_pwm){;}

    void setTargetWatt(const real_t watt){
        target_watt = watt;
    }

    void run(){
        ina.update();
        
        power_ctrl.update(target_watt, ina.getPower(), ina.getCurrent());
        pwm = output;
    }
};

}
#endif