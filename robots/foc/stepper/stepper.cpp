#include "stepper.hpp"
#include "types/float/bf16.hpp"

static auto & nozzle_en_gpio = portA[0];

void FOCStepper::setNozzle(const real_t duty){
    nozzle_en_gpio.outpp();
    nozzle_en_gpio = bool(duty);
}

uint8_t FOCStepper::getDefaultNodeId(){
    auto chip_id = Sys::Chip::getChipIdCrc();
    switch(chip_id){
        case 3273134334:
            return 3;
        case 341554774:
            return 2;
        case 4079188777:
            return 1;
        case 0x551C4DEA:
            return  3;
        default:
            return 0;
    }
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
                        active_task(true);
                    }else{
                        beep_task(true);
                    }
                }else{
                    cali_task(true);
                }
                break;
            }

        case RunStatus::CHECK:
            exe_status = check_task();
            break;

        case RunStatus::CALI:
            exe_status = cali_task();
            break;

        case RunStatus::ACTIVE:
            exe_status = active_task();
            break;

        case RunStatus::BEEP:
            exe_status = beep_task();
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
                    cali_task(true);
                    break;
                case RunStatus::CALI:
                    if(skip_tone){
                        active_task(true);
                    }
                    else beep_task(true);
                    break;
                case RunStatus::BEEP:
                    active_task(true);
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
                    check_task(true);
                    break;
                case RunStatus::CALI:
                    cali_task(true);
                    break;
                case RunStatus::ACTIVE:
                    active_task(true);
                    break;
                case RunStatus::BEEP:
                    beep_task(true);
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
    readCan();
    panel_led.run();

    #ifndef STEPPER_NO_PRINT
    {
        static String temp;
        while(logger.available()){
            auto chr = logger.read();
            if(chr == 0) continue;
            temp += chr;
            if(chr == '\n'){
                temp.alphanum();
                parseLine(temp);
                temp = "";
            }
        }
    }
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
    if(logger.pending()==0){
        // delayMicroseconds(200);   
        // delay(1); 
        RUN_DEBUG(std::setprecision(4), target, getSpeed(), getPosition(), getCurrent(), run_leadangle,std::setprecision(4), getPositionErr());
    }
    // delay(1);
    // , est_speed, t, odo.getElecRad(), openloop_elecrad);
    // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
    // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
    // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

    // bool led_status = (millis() / 200) % 2;
    // bled = led_status;
}