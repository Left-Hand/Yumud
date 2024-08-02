#include "../robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    if(ctrl_type != CtrlType::VECTOR) run_elecrad = est_elecrad + run_leadangle;
    else run_elecrad = odo.position2rad(target);

    setCurrent(curr_ctrl.update(measurements.curr), run_elecrad + elecrad_zerofix);

    odo.update();

    measurements.pos = odo.getPosition();
    est_elecrad = odo.getElecRad();
    measurements.spd = (speed_estmator.update(measurements.pos) + measurements.spd * 127) >> 7;

    if(init_flag){

        run_status = RunStatus::ACTIVE;

        setCurrent(real_t(0), real_t(0));
        return RunStatus::NONE;
    }

    // if(auto_shutdown_activation){
    //     if(est_current){
    //         auto_shutdown_actived = false;
    //         wakeup();
    //         auto_shutdown_last_wake_ms = millis();
    //     }else{
    //         if(millis() - auto_shutdown_last_wake_ms > auto_shutdown_timeout_ms){
    //             auto_shutdown_actived = true;
    //             shutdown();
    //             auto_shutdown_last_wake_ms = millis();
                
    //         }
    //     }
    // }



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
                result = {curr_ctrl.config.current_clamp, 0};
                break;
            case CtrlType::POSITION:
                result = position_ctrl.update(target_position_clamp.clamp(target), est_pos, est_spd, est_elecrad);
                break;
            case CtrlType::TRAPEZOID:
                result = trapezoid_ctrl.update(target, est_pos, est_spd, est_elecrad);
                break;
            case CtrlType::SPEED:
                result = speed_ctrl.update(target, est_spd);
                break;
        } 

        measurements.curr = result.current;
        run_leadangle = result.raddiff;
    }



    return RunStatus::NONE;
}