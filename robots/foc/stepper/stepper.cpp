#include "stepper.hpp"
#include "sys/math/float/bf16.hpp"

static auto & nozzle_en_gpio = portA[0];

void FOCStepper::setNozzle(const real_t duty){
    nozzle_en_gpio.outpp();
    nozzle_en_gpio = bool(duty);
}

void FOCStepper::invoke_cali(){
    cali_tasker.reset();
    run_status = RunStatus::CALI;
}
void FOCStepper::invoke_tone_task(){
    tone_tasker.reset();
    run_status = RunStatus::BEEP;
}

void FOCStepper::invoke_active_task(){
    run_status = RunStatus::ACTIVE;
    svpwm.setDuty(real_t(0), real_t(0));
}

void FOCStepper::invoke_selfcheck_task(){
    selfcheck_tasker.reset();
    run_status = RunStatus::CHECK;
}
void FOCStepper::tick(){
    auto begin_micros = micros();
    RunStatus exe_status = RunStatus::NONE;

    switch(run_status){
        case RunStatus::INIT:
            {
                bool load_ok = loadArchive(false);
                if(load_ok){
                    if(skip_tone){
                        invoke_active_task();
                    }else{
                        invoke_tone_task();
                    }
                }else{
                    invoke_cali();
                }
                break;
            }

        case RunStatus::CHECK:
            selfcheck_tasker.run();
            exe_status = selfcheck_tasker.done() ? RunStatus::EXIT : RunStatus::NONE;
            break;

        case RunStatus::CALI:
            cali_tasker.run();
            exe_status = cali_tasker.done() ? RunStatus::EXIT : RunStatus::NONE;
            break;

        case RunStatus::ACTIVE:
            active_task();
            exe_status = RunStatus::NONE;
            break;

        case RunStatus::BEEP:
            tone_tasker.run();
            exe_status = tone_tasker.done() ? RunStatus::EXIT : RunStatus::NONE;
            break;

        case RunStatus::INACTIVE:
            run_status = RunStatus::ACTIVE;
            break;

        default:
            break;
    }

    //decide next status by execution result 

    if(not (exe_status == (RunStatus::NONE))){//execution meet sth.

        if((exe_status == RunStatus::ERROR)){
        }

        else if((exe_status == RunStatus::EXIT)){
            switch(run_status){
                case RunStatus::CHECK:
                    invoke_cali();
                    break;
                case RunStatus::CALI:
                    if(skip_tone){
                        invoke_active_task();
                    }
                    else{
                        invoke_tone_task();
                    }
                    break;
                case RunStatus::BEEP:
                    invoke_active_task();
                    break;
                case RunStatus::ACTIVE:
                    break;
                case RunStatus::INACTIVE:
                    break;
                case RunStatus::ERROR:
                    break;
                case RunStatus::WARN:
                    break;
                default:
                break;
            }
        }else{
            switch(run_status){
                case RunStatus::CHECK:
                    invoke_selfcheck_task();
                    break;
                case RunStatus::CALI:
                    invoke_cali();
                    break;
                case RunStatus::ACTIVE:
                    invoke_active_task();
                    break;
                case RunStatus::BEEP:
                    invoke_tone_task();
                    break;
                case RunStatus::INACTIVE:
                    break;
                case RunStatus::ERROR:
                    break;
                case RunStatus::WARN:
                    break;
                default:
                    break;
            }
        }
    }
    exe_micros = micros() - begin_micros;
}

void FOCStepper::run(){
    if(can_protocol) can_protocol->readCan();
    panel_led.run();

    #ifndef STEPPER_NO_PRINT
    if(ascii_protocol) ascii_protocol->readString();
    #endif

    red_pwm.tick();
    green_pwm.tick();
    blue_pwm.tick();
}

void FOCStepper::report(){
    // real_t total = real_t(3);
    // static real_t freq = real_t(10);
    // static real_t freq_dir = real_t(1);
    // const real_t freq_delta = real_t(20);
    // if(freq > real_t(300)) freq_dir = real_t(-1);
    // else if(freq < real_t(4)) freq_dir = real_t(1);
    // static real_t last_t = t;
    // real_t delta = (t - last_t);
    // freq += delta * freq_dir * freq_delta;
    // last_t = t;
    // static real_t ang = real_t(0);
    // ang += freq * delta;
    // real_t target = (total / freq) * sin(ang);

    // target_pos = sign(frac(t) - 0.5);
    // target_pos = sin(t);
    // RUN_DEBUG(, est_pos, est_speed);
    if(DEBUGGER.pending()==0){
    //     // delayMicroseconds(200);   
    //     // delay(1); 
        DEBUG_PRINTLN(std::setprecision(3), target, getSpeed(), getPosition(), getCurrent(), run_leadangle, speed_ctrl.soft_targ_spd);
    }
    // delay(1);
    // , est_speed, t, odo.getElecRad(), openloop_elecrad);
    // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
    // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
    // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

    // bool led_status = (millis() / 200) % 2;
    // bled = led_status;
}