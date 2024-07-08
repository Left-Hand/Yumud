#include "robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    // auto target = sign(frac(t) - 0.5);
    // auto target = floor(t);
// auto target=sin(t);
    // real_t raw_current = 0.1 * sin(t);
    // run_current = abs(raw_current);
    // run_leadangle = SIGN_AS(PI / 2, raw_current);


    if(ctrl_type != CtrlType::VECTOR) run_elecrad = est_elecrad + run_leadangle;
    else run_elecrad = odo.position2rad(target);

    setCurrent(curr_ctrl.update(run_current), run_elecrad + elecrad_zerofix);
    // setCurrent(2, est_elecrad + PI / 2);



    // coilB = 0.1 * sin(run_elecrad);
    // coilA = 0.1 * cos(run_elecrad);
    // run_elecrad = est_elecrad + PI * 0.5; setCurrent(0.02, run_elecrad + elecrad_zerofix);//n = 2
    // run_elecrad = est_elecrad + PI * 0.5; setCurrent(0.3, TAU * frac(t));//n = 2


    // setCurrent(0.2, odo.position2rad(target));
    // setCurrent(0.4, TAU * frac(t));
    // setCurrent(0, 0);

    // coilB = (0.2);
    // coilA = 0.2 * sin(t);
    // uint32_t foc_begin_micros = nanos();
    odo.update();

    raw_pos = odo.getPosition();
    est_pos = raw_pos;
    est_elecrad = odo.getElecRad();
    static SpeedEstimator speed_estmator;
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
        HighLayerCtrl::Result result;

        switch(ctrl_type){
            case CtrlType::CURRENT:
                result = {ABS(target), SIGN_AS(PI / 2, target)};
                break;
            case CtrlType::VECTOR:
                result = {openloop_current, 0};
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