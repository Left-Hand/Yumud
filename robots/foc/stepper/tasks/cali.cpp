#include "robots/foc/stepper/stepper.hpp"
#include "cali.hpp"

#include <numeric>





void CaliTasker::run(){
    {
        switch(sub_state){
            case SubState::ENTRY:
                sw_state(SubState::ALIGN);
                break;

            case SubState::ALIGN:
                svpwm.setDuty(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    odo.reset();
                    odo.update();
                    sw_state(SubState::PRE_FORWARD);
                }
                break;

            case SubState::PRE_FORWARD:

                svpwm.setDuty(cali_current, real_t(cnt % subdivide_micros) * tau / subdivide_micros + pi_2);

                if(cnt >= forward_precycles * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();
                    tracker.reset();
                    sw_state(SubState::FORWARD);
                }
                break;
            case SubState::FORWARD:
                odo.update();

                svpwm.setDuty(cali_current, real_t(cnt % subdivide_micros) * tau / subdivide_micros + pi_2);

                if(cnt % subdivide_micros == 0){

                    const uint cali_index = warp_mod(openloop_pole++, poles);

                    
                    real_t err = tracker.update(odo.getRawLapPosition());

                    forward_pole_err[cali_index] += err / (forward_cycles / poles);
                }

                if(cnt >= forward_cycles * subdivide_micros){
                    sw_state(SubState::REALIGN);
                }
                break;

            case SubState::REALIGN:
                svpwm.setDuty(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::PRE_BACKWARD);
                }
                break;

            case SubState::PRE_BACKWARD:

                svpwm.setDuty(cali_current, -real_t(cnt % subdivide_micros) * tau / subdivide_micros - pi_2);

                if(cnt >= backward_precycles * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();
                    tracker.reset();
                    sw_state(SubState::BACKWARD);
                }
                break;

            case SubState::BACKWARD:
                odo.update();

                svpwm.setDuty(cali_current, -real_t(cnt % subdivide_micros) * tau / subdivide_micros - pi_2);

                if(cnt % subdivide_micros == 0){
                    const uint cali_index = warp_mod(openloop_pole--, poles);

                    real_t err = tracker.update(odo.getRawLapPosition());

                    backward_pole_err[cali_index] += err / (backward_cycles / poles);
                }

                if(cnt >= backward_cycles * subdivide_micros){
                    sw_state(SubState::STOP);
                    openloop_pole = 0;
                }
                break;
            case SubState::STOP:
                odo.update();

                svpwm.setDuty(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::ANALYSIS);
                }

                
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
                        
                    // }else{
                    //     for(uint8_t i = 0; i < poles; i++){
                    //         odo.map()[i] = mean(forward_pole_err[i], backward_pole_err[i]);
                    //     }
                    // }
                    // // // initial_err -= forward_err[initial_pole];
                    // for(size_t p = 0; p < poles; p++){
                    //     size_t i = p % poles;
                    //     // logger << forward_test_data[i].first << ", " << forward_test_data[i].second << ", " << forward_err[i] << ", " << backward_test_data[i].first << ", " << backward_test_data[i].second << ", " <<  backward_err[i] << "\r\n";
                    //     // logger.println(odo.map()[i], forward_pole_err[i], backward_pole_err[i], forward_mean, backward_mean);
                    //     delay(1);
                    // }
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