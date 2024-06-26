#include "robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    // auto target = sign(frac(t) - 0.5);
    // auto target = floor(t);
// auto target=sin(t);
    // real_t raw_current = 0.1 * sin(t);
    // run_current = abs(raw_current);
    // run_leadangle = SIGN_AS(PI / 2, raw_current);


    if(+ctrl_type != +CtrlType::VECTOR) run_elecrad = est_elecrad + run_leadangle;
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
    static uint32_t est_cnt;


    if(init_flag){
        est_pos = raw_pos;
        est_speed = real_t();
        est_cnt = 0;
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


    est_pos = raw_pos;
    est_elecrad = odo.getElecRad();

    {
        HighLayerCtrl::Result result;

        switch(ctrl_type){
            case CtrlType::VECTOR:
                result = {0.6, 0};
                break;
            case CtrlType::POSITION:
                result = position_ctrl.update(target, est_pos, est_speed, est_elecrad);
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



    

    {//estimate speed and update controller
        // static real_t est_delta_raw_pos_intergal = real_t();
        static SpeedEstimator speed_estmator;
        est_cnt++;
        if(est_cnt%est_devider == 0){ // est happens
            est_speed = (speed_estmator.update(raw_pos) + est_speed * 31) >> 5;
            // est_speed = speed_estmator.update(raw_pos);
            // if(true){
            //     switch(CTZ(MAX(int(abs(est_speed_new)), 1))){
            //         case 0://  1r/s
            //             est_speed = (est_speed_new + est_speed * 63) >> 6;
            //             break;
            //         case 1://  2r/s
            //             est_speed = (est_speed_new + est_speed * 15) >> 4;
            //             break;
            //         case 2:// 4r/s
            //             // est_speed = (est_speed_new + est_speed * 31) >> 5;
            //             // break;
            //         case 3:// 8r/s
            //             // est_speed = (est_speed_new + est_speed * 15) >> 4;
            //             // break;
            //         case 4:// 16r/s
            //             est_speed = (est_speed_new + est_speed * 7) >> 3;
            //             break;
            //         case 5:// 32r/s
            //             // est_speed = (est_speed_new + est_speed * 3) >> 2;
            //             // break;
            //         default:
            //         case 6:// 64r/s or more
            //             est_speed = (est_speed_new + est_speed) >> 1;
            //             break;
            //     }
            // }else{
            //     est_speed = est_speed_new;
            // }
            // est_delta_raw_pos_intergal = real_t();
            // est_cnt = 0;

            // run_current = 0.2;
            // run_leadangle = -PI / 2;
        }
    }



    // uint32_t foc_end_micros = nanos();
    // foc_pulse_micros = foc_end_micros - foc_begin_micros;

    {
        // struct SpeedCtrl{
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
        // };
    }

    return RunStatus::NONE;
}