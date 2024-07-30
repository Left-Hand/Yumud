#include "../robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    if(ctrl_type != CtrlType::VECTOR) run_elecrad = est_elecrad + run_leadangle;
    else run_elecrad = odo.position2rad(target);

    setCurrent(curr_ctrl.update(run_current), run_elecrad + elecrad_zerofix);

    odo.update();

    raw_pos = odo.getPosition();
    est_pos = raw_pos;
    est_elecrad = odo.getElecRad();
    est_speed = (speed_estmator.update(raw_pos) + est_speed * 127) >> 7;

    if(init_flag){
        est_pos = raw_pos;
        est_speed = real_t();

        run_status = RunStatus::ACTIVE;

        setCurrent(real_t(0), real_t(0));
        return RunStatus::NONE;
    }

    // if(auto_shutdown_activation){
    //     if(run_current){
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

        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(target), SIGN_AS(PI / 2, target)};
                break;
            case CtrlType::VECTOR:
                result = {curr_ctrl.config.current_clamp, 0};
                break;
            case CtrlType::POSITION:
                result = position_ctrl.update(target_position_clamp.clamp(target), est_pos, est_speed, est_elecrad);
                break;
            case CtrlType::TRAPEZOID:
                result = trapezoid_ctrl.update(target, est_pos, est_speed, est_elecrad);
                break;
            case CtrlType::SPEED:
                result = speed_ctrl.update(target, est_speed);
                break;
        } 

        run_current = result.current;
        run_leadangle = result.raddiff;
    }



    return RunStatus::NONE;
}