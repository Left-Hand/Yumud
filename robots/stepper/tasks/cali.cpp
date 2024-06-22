#include "robots/stepper/stepper.hpp"
#include <numeric>

Stepper::RunStatus Stepper::cali_task(const Stepper::InitFlag init_flag){
    enum class SubState{
        ENTRY,
        ALIGN,
        INIT,
        PRE_FORWARD,
        FORWARD,
        BREAK,
        REALIGN,
        PRE_BACKWARD,
        BACKWARD,
        PRE_LANDING,
        LANDING,
        STOP,
        ANALYSIS,
        EXAMINE,
        DONE
    };

    constexpr int forwardpreturns = 15;
    constexpr int forwardturns = 100;
    constexpr int backwardpreturns = forwardpreturns;
    constexpr int backwardturns = forwardturns;

    constexpr int subdivide_micros = 256;
    constexpr int cogging_samples = 16;
    constexpr int align_ms = 200;

    constexpr float cali_current = 0.6;
    constexpr float align_current = 0.6;


    static SubState sub_state = SubState::DONE;
    static uint32_t cnt = 0;
    static int openloop_pole = 0;

    static std::array<real_t, 50> forward_pole_err;
    static std::array<real_t, 50> backward_pole_err;

    static std::array<real_t, cogging_samples> forward_cogging_err;
    static std::array<real_t, cogging_samples> backward_cogging_err;

    if(init_flag){
        cnt = 0;
        sub_state = SubState::ALIGN;
        openloop_pole = 0;
        forward_pole_err.fill(0);
        backward_pole_err.fill(0);
        forward_cogging_err.fill(0);
        backward_cogging_err.fill(0);
        run_status = RunStatus::CALI;
        return RunStatus::NONE;
    }

    auto sw_state = [](const SubState & new_state){
        sub_state = new_state;
        cnt = 0;
    };

    // auto accumulate_raw_position = [](const real_t & raw_lap_position){
    //     real_t deltaLapPosition = raw_lap_position - last_raw_lap_position;
    //     if(deltaLapPosition > real_t(0.5f)){
    //         deltaLapPosition -= real_t(1);
    //     }else if (deltaLapPosition < real_t(-0.5f)){
    //         deltaLapPosition += real_t(1);
    //     }

    //     raw_position_accumulate += deltaLapPosition;
    //     last_raw_lap_position = raw_lap_position;
    // };

    {

        switch(sub_state){
            case SubState::ALIGN:
                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * 500)){
                    sw_state(SubState::PRE_FORWARD);
                    odo.reset();
                    odo.inverse();
                    odo.update();
                }
                break;

            case SubState::PRE_FORWARD:

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU + PI / 2);

                if(cnt >= forwardpreturns * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::FORWARD);
                }
                break;
            case SubState::FORWARD:
                odo.update();

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU + PI/2);

                if(cnt % subdivide_micros == 0){
                    openloop_pole++;

                    const uint8_t cali_index =warp_mod(openloop_pole, 50);

                    static real_t last_err = 0;

                    real_t err_a = fmod(odo.getRawLapPosition(), 0.02);
                    real_t err_b = err_a - 0.02;

                    last_err = ((ABS(err_b - last_err) > ABS(err_a - last_err))) ? err_a : err_b;
                    // last_err = err_a;

                    forward_pole_err[cali_index] += last_err / (forwardturns / 50);
                    
                    // if(cnt % (subdivide_micros / cogging_samples) == 0){
                    //     forward_cogging_err[cnt / (subdivide_micros / cogging_samples)]
                    // }
                }

                if(cnt >= forwardturns * subdivide_micros){
                    sw_state(SubState::REALIGN);
                }
                break;

            case SubState::REALIGN:
                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::PRE_BACKWARD);
                }
                break;

            case SubState::PRE_BACKWARD:

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU - PI / 2);

                if(cnt >= backwardpreturns * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::BACKWARD);
                }
                break;

            case SubState::BACKWARD:
                odo.update();

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU - PI/2);

                if(cnt % subdivide_micros == 0){
                    openloop_pole--;

                    const uint8_t cali_index = warp_mod(openloop_pole, 50);

                    static real_t last_err = 0.1;

                    real_t err_a = fmod(odo.getRawLapPosition(), 0.02);
                    real_t err_b = err_a - 0.02;

                    last_err = ((ABS(err_b - last_err) > ABS(err_a - last_err))) ? err_a : err_b;
                    // last_err = err_a;

                    backward_pole_err[cali_index] += last_err / (backwardturns / 50);
                    // backward_err[cali_index] = odo.getRawLapPosition();
                }

                if(cnt >= backwardturns * subdivide_micros){
                    sw_state(SubState::STOP);
                    openloop_pole = 0;
                }
                break;
            
            // case SubState::PRE_LANDING:
            //     odo.update();

            //     setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);
            //     if(cnt >= landingpreturns * subdivide_micros){
            //         openloop_pole = 0;

            //         sw_state(SubState::LANDING);
            //     }
            //     break;

            // case SubState::LANDING:
            //     odo.update();

            //     setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);
            //     if(cnt % subdivide_micros == 0){
            //         elecrad_test_data[openloop_pole].first = odo.getLapPosition();
            //         elecrad_test_data[openloop_pole].second = odo.getElecRad();
            //         openloop_pole++;
            //     }

            //     if(cnt >= landingturns * subdivide_micros){
            //         sw_state(SubState::STOP);
            //     }
            //     break;
            case SubState::STOP:
                odo.update();

                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::ANALYSIS);
                }

                
                break;
            case SubState::ANALYSIS:
                {
                    real_t forward_mean = std::accumulate(std::begin(forward_pole_err), std::end(forward_pole_err), real_t(0)) / poles;
                    real_t backward_mean = std::accumulate(std::begin(backward_pole_err), std::end(backward_pole_err), real_t(0)) / poles;

                    // maybe forward is delayed by a inv_poles
                    // assume inv_poles is 0.2
                    // for example:

                    // backward mean -0.03 
                    // forward mean -0.13
                    // final -0.08

                    // but the fact is that
                    // forward mean 0.07
                    // backward mean -0.03
                    // final 0.02

                    // we will fix that

                    // if(ABS((forward_mean - backward_mean) - inv_poles / 2) < ABS((forward_mean + inv_poles - backward_mean) - inv_poles / 2)){
                    //     for(auto & item : forward_pole_err) item += inv_poles;
                    // }

                    if(forward_mean < backward_mean){
                        for(auto & item : forward_pole_err) item += inv_poles;
                    }

                    bool load_ok = autoload();
                    if(load_ok){
                        
                    }else{
                        for(uint8_t i = 0; i < poles; i++){
                            odo.map()[i] = mean(forward_pole_err[i], backward_pole_err[i]);
                        }
                    }
                    // initial_err -= forward_err[initial_pole];
                    for(size_t p = 0; p < poles; p++){
                        // size_t i = p % 50;
                        // logger << forward_test_data[i].first << ", " << forward_test_data[i].second << ", " << forward_err[i] << ", " << backward_test_data[i].first << ", " << backward_test_data[i].second << ", " <<  backward_err[i] << "\r\n";
                        // logger.println(odo.map()[i], forward_pole_err[i], backward_pole_err[i], forward_mean, backward_mean);
                        // delay(1);
                    }
                }



                sw_state(SubState::EXAMINE);
                break;
            case SubState::EXAMINE:

                odo.locateRelatively(real_t(0));
                setCurrent(real_t(0), real_t(0));
                sw_state(SubState::DONE);
                break;

            case SubState::DONE:
                return RunStatus::EXIT;
                break;
            default:
                break;
        }
        cnt++;
    }
    return RunStatus::NONE;
}