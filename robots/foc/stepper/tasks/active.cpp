#include "robots/foc/stepper/stepper.hpp"


FOCStepper::RunStatus FOCStepper::active_task(const FOCStepper::InitFlag init_flag){
    if(ctrl_type == CtrlType::VECTOR){
        run_elecrad = odo.position2rad(target);
        svpwm.setCurrent(curr_ctrl.config.openloop_curr, run_elecrad + elecrad_zerofix);
    }else{
        run_elecrad = est_elecrad + curr_ctrl.raddiff_output;
        svpwm.setCurrent(curr_ctrl.current_output, run_elecrad + elecrad_zerofix);
    }

    odo.update();

    measurements.pos = odo.getPosition();
    est_elecrad = odo.getElecRad();
    static real_t temp;
    measurements.spd = (speed_estmator.update(measurements.pos) + measurements.spd * 127) >> 7;
    
    if(init_flag){
        run_status = RunStatus::ACTIVE;
        svpwm.setCurrent(real_t(0), real_t(0));
        return RunStatus::NONE;
    }

    {
        using Result = CtrlResult;
        Result result;

        const auto & est_pos = measurements.pos;
        const auto & est_spd = measurements.spd;

        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(target), SIGN_AS(real_t(PI / 2) *real_t(1.3), target)};
                break;
            case CtrlType::VECTOR:
                result = {ctrl_limits.max_curr, 0};
                break;
            case CtrlType::POSITION:
                result = position_ctrl.update(target, est_pos, est_spd, est_elecrad);
                break;
            case CtrlType::TRAPEZOID:
                result = trapezoid_ctrl.update(target, est_pos, est_spd, est_elecrad);
                break;
            case CtrlType::SPEED:
                result = speed_ctrl.update(target, est_spd);
                break;
            case CtrlType::TEACH:{
                real_t max_current = target;
                real_t spd = getSpeed();
                real_t abs_spd = ABS(spd);
                static constexpr real_t deadzone = real_t(0.23);
                if(abs_spd < deadzone){
                    result = {0, 0}; 
                }else{
                    real_t expect_current = MIN(abs_spd * real_t(0.27), max_current);
                    result = {expect_current, SIGN_AS(real_t(PI / 2) *real_t(1.3), spd)};
                }
                break;
            }
        } 

        curr_ctrl.update(result);
        measurements.curr = curr_ctrl.current_output;
        run_leadangle = curr_ctrl.raddiff_output;
    }



    return RunStatus::NONE;
}