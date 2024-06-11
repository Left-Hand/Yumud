#include "robots/stepper/stepper.hpp"


Stepper::RunStatus Stepper::active_task(const Stepper::InitFlag init_flag){
    // auto target = sign(frac(t) - 0.5);
    auto target = floor(t);
// auto target=sin(t);
    // real_t raw_current = 0.1 * sin(t);
    // run_current = abs(raw_current);
    // run_raddiff = SIGN_AS(PI / 2, raw_current);

    setCurrent(curr_ctrl.update(run_current), (run_elecrad = est_elecrad + run_raddiff) + elecrad_zerofix);

    // uint32_t foc_begin_micros = nanos();
    odo.update();

    raw_pos = odo.getPosition();

    static real_t last_raw_pos;
    static uint32_t est_cnt;


    if(init_flag){
        est_pos = raw_pos;
        est_speed = real_t();
        last_raw_pos = raw_pos;
        est_cnt = 0;
        run_status = RunStatus::ACTIVE;

        setCurrent(real_t(0), real_t(0));
        return RunStatus::NONE;
    }

    if(auto_shutdown_activation){
        if(run_current){
            auto_shutdown_actived = false;
            wakeup();
            auto_shutdown_last_wake_ms = millis();
        }else{
            if(millis() - auto_shutdown_last_wake_ms > auto_shutdown_timeout_ms){
                auto_shutdown_actived = true;
                shutdown();
                auto_shutdown_last_wake_ms = millis();
                
            }
        }
    }

    real_t delta_raw_pos = raw_pos - last_raw_pos;
    last_raw_pos = raw_pos;

    est_pos = raw_pos + delta_raw_pos;
    est_elecrad = odo.getElecRad();
    // Fixed();

    {//estimate speed and update controller
        static real_t est_delta_raw_pos_intergal = real_t();

        est_cnt++;
        if(est_cnt == est_devider){ // est happens
            real_t est_speed_new = est_delta_raw_pos_intergal * (int)est_freq;

            est_speed = (est_speed_new + est_speed * 31) >> 5;

            est_delta_raw_pos_intergal = real_t();
            est_cnt = 0;


            // auto result = speed_ctrl.update(target, est_speed);
            auto result = position_ctrl.update(target, est_pos, est_speed, est_elecrad);

            run_current = result.current;
            run_raddiff = result.raddiff;
        }else{
            est_delta_raw_pos_intergal += delta_raw_pos;
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