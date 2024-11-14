#include "cali.hpp"

#include "robots/foc/stepper/stepper.hpp"
#include <bits/stl_numeric.h>

using namespace ymd::foc;
void CaliTasker::run(){

    auto get_rad = [this](const bool is_forward) -> real_t{
        auto abs_ret = (real_t(cnt % subdivide_micros) * tau) / subdivide_micros + pi_2;
        return is_forward ? abs_ret : -abs_ret;
    };
    auto align_func = [this](const SubState next_state){
        svpwm.setDuty(real_t(align_duty), real_t(0));
        if(cnt >= (int)((foc_freq / 1000) * align_ms)){
            odo.reset();
            odo.update();
            sw_state(next_state);
        }
    };

    auto preturn_func = [&](const SubState next_state){
        svpwm.setDuty(cali_duty, get_rad((uint8_t)next_state == (uint8_t)SubState::PRE_FORWARD + 1));

        if(cnt >= forward_precycles * subdivide_micros){
            odo.update();
            openloop_pole = odo.getRawPole();
            tracker.reset();
            sw_state(next_state);
        }
    };

    auto turn_func = [&](const SubState next_state){
        odo.update();
        bool is_forward = (uint8_t)next_state == (uint8_t)SubState::FORWARD + 1;
        svpwm.setDuty(cali_duty, get_rad(is_forward));

        if(cnt % subdivide_micros == 0){
            const uint cali_index = warp_mod(openloop_pole, poles);

            if(is_forward) openloop_pole++;
            else openloop_pole--;
            real_t err = tracker.update(odo.getRawLapPosition());

            auto & err_map = is_forward ? forward_pole_err : backward_pole_err;
            
            err_map[cali_index] += err * poles / forward_cycles;
        }

        if(cnt >= forward_cycles * subdivide_micros){
            sw_state(next_state);
        }
    };
    
    switch(sub_state){
        case SubState::ENTRY:
            sw_state(SubState::ALIGN);
            break;

        case SubState::ALIGN:
            align_func(SubState::PRE_FORWARD);
            break;

        case SubState::PRE_FORWARD:
            preturn_func(SubState::FORWARD);
            break;

        case SubState::FORWARD:
            turn_func(SubState::REALIGN);
            break;

        case SubState::REALIGN:
            align_func(SubState::PRE_BACKWARD);
            break;

        case SubState::PRE_BACKWARD:
            preturn_func(SubState::BACKWARD);
            break;

        case SubState::BACKWARD:
            turn_func(SubState::STOP);
            break;

        case SubState::STOP:
            align_func(SubState::ANALYSIS);
            break;

        case SubState::ANALYSIS:
            {
                real_t forward_mean = std::accumulate(std::begin(forward_pole_err), std::end(forward_pole_err), real_t(0)) / poles;
                real_t backward_mean = std::accumulate(std::begin(backward_pole_err), std::end(backward_pole_err), real_t(0)) / poles;

                if(forward_mean < backward_mean){
                    for(auto & item : forward_pole_err) item += inv_poles;
                }

                for(uint8_t i = 0; i < poles; i++){
                    odo.map()[i] = mean(forward_pole_err[i], backward_pole_err[i]);
                }
            }
            sw_state(SubState::EXAMINE);
            break;

        case SubState::EXAMINE:
            sw_state(SubState::DONE);
            break;

        case SubState::DONE:
            svpwm.setDuty(real_t(0), real_t(0));

        default:
            break;
    }
    cnt++;
}

bool CaliTasker::done(){
    return sub_state == SubState::DONE;
}

void CaliTasker::reset(){
    sw_state(SubState::ENTRY);
    openloop_pole = 0;
    forward_pole_err.fill(0);
    backward_pole_err.fill(0);
    forward_cogging_err.fill(0);
    backward_cogging_err.fill(0);
}