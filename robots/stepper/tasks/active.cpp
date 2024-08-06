#include "../robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    if(ctrl_type == CtrlType::VECTOR){
        run_elecrad = odo.position2rad(target);
    }else{
        run_elecrad = est_elecrad + curr_ctrl.raddiff_output;
    }
    setCurrent(curr_ctrl.current_output, run_elecrad + elecrad_zerofix);

    odo.update();

    measurements.pos = odo.getPosition();
    est_elecrad = odo.getElecRad();
    measurements.spd = (speed_estmator.update(measurements.pos) + measurements.spd * 127) >> 7;

    if(init_flag){

        run_status = RunStatus::ACTIVE;

        setCurrent(real_t(0), real_t(0));
        return RunStatus::NONE;
    }

    {
        using Result = CtrlResult;
        Result result;

        const auto & est_pos = measurements.pos;
        const auto & est_spd = measurements.spd;

        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(target), SIGN_AS(PI / 2, target)};
                break;
            case CtrlType::VECTOR:
                result = {curr_ctrl.config.curr_limit, 0};
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
        } 

        curr_ctrl.update(result);
        measurements.curr = curr_ctrl.current_output;
        run_leadangle = curr_ctrl.raddiff_output;
    }



    return RunStatus::NONE;
}