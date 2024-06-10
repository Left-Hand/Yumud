#include "../stepper.hpp"
#include <numeric>

Stepper::ExitFlag Stepper::cali_task(const Stepper::InitFlag init_flag){
    enum class SubState{
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
    constexpr int forwardturns = 50;
    constexpr int backwardpreturns = forwardpreturns;
    constexpr int backwardturns = forwardturns;

    constexpr int landingpreturns = 15;
    constexpr int landingturns = 50;

    constexpr int subdivide_micros = 256;
    constexpr int align_ms = 100;
    constexpr int break_ms = 100;
    constexpr int stop_ms = 400;

    constexpr float cali_current = 0.3;
    constexpr float align_current = 0.4;


    static SubState sub_state = SubState::DONE;
    static uint32_t cnt = 0;
    // static real_t openloop_elecrad_step = real_t(TAU / subdivide_micros);

    static real_t raw_position_accumulate = real_t(0);
    static real_t last_raw_lap_position = real_t(0);

    static real_t landing_position_accumulate = real_t(0);

    // using real = real_t;

    static int openloop_pole;

    static std::array<std::pair<real_t, real_t>, 50> forward_test_data;
    static std::array<std::pair<real_t, real_t>, 50> backward_test_data;
    static std::array<real_t, 50> elecrad_test_data;

    static std::array<real_t, 50> forward_err;
    static std::array<real_t, 50> backward_err;

    if(init_flag){
        sub_state = SubState::ALIGN;
        cnt = 0;
        return false;
    }

    auto sw_state = [](const SubState & new_state){
        sub_state = new_state;
        cnt = 0;
    };

    auto accumulate_raw_position = [](const real_t & raw_lap_position){
        real_t deltaLapPosition = raw_lap_position - last_raw_lap_position;
        if(deltaLapPosition > real_t(0.5f)){
            deltaLapPosition -= real_t(1);
        }else if (deltaLapPosition < real_t(-0.5f)){
            deltaLapPosition += real_t(1);
        }

        raw_position_accumulate += deltaLapPosition;
        last_raw_lap_position = raw_lap_position;
    };

    if(sub_state == SubState::DONE){
        return true;
    }

    else{

        switch(sub_state){
            case SubState::ALIGN:
                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::INIT);
                }
                break;
            case SubState::INIT:
                odo.reset();
                odo.inverse();
                odo.update();

                sw_state(SubState::PRE_FORWARD);
                break;

            case SubState::PRE_FORWARD:

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                if(cnt >= forwardpreturns * subdivide_micros){
                    odo.update();
                    raw_position_accumulate = real_t(0);
                    last_raw_lap_position = odo.getRawLapPosition();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::FORWARD);
                }
                break;
            case SubState::FORWARD:
                odo.update();
                accumulate_raw_position(odo.getRawLapPosition());

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                if(cnt % subdivide_micros == 0){
                    openloop_pole++;

                    const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
                    real_t openloop_position = real_t(cnt / subdivide_micros) * real_t(inv_poles);

                    forward_test_data[cali_index].first = openloop_position;
                    forward_test_data[cali_index].second = raw_position_accumulate;

                    forward_err[cali_index] = openloop_position - raw_position_accumulate;
                }

                if(cnt >= forwardturns * subdivide_micros){
                    sw_state(SubState::BREAK);
                }
                break;
            case SubState::BREAK:
                setCurrent(real_t(cali_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * break_ms)){
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

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                if(cnt >= backwardpreturns * subdivide_micros){
                    odo.update();
                    raw_position_accumulate = real_t(0);
                    last_raw_lap_position = odo.getRawLapPosition();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::BACKWARD);
                }
                break;

            case SubState::BACKWARD:
                odo.update();
                accumulate_raw_position(odo.getRawLapPosition());

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / real_t(subdivide_micros) * TAU);

                if(cnt % subdivide_micros == 0){
                    openloop_pole--;

                    const uint8_t cali_index = odo.warp_mod(openloop_pole, 50);
                    real_t openloop_position = -real_t(cnt / subdivide_micros) * real_t(inv_poles);

                    backward_test_data[cali_index].first = openloop_position;
                    backward_test_data[cali_index].second = raw_position_accumulate;

                    backward_err[cali_index] = openloop_position - raw_position_accumulate;
                }

                if(cnt >= backwardturns * subdivide_micros){
                    real_t forward_mean = std::accumulate(std::begin(forward_err), std::end(forward_err), real_t(0)) / int(forward_err.size());
                    for(auto & forward_err_item : forward_err){
                        forward_err_item -= forward_mean;
                    }

                    real_t backward_mean = std::accumulate(std::begin(backward_err), std::end(backward_err), real_t(0)) / int(backward_err.size());
                    for(auto & backward_err_item : backward_err){
                        backward_err_item -= backward_mean;
                    }

                    for(uint8_t i = 0; i < poles; i++){
                        odo.cali_map[i] = mean(forward_err[i], backward_err[i]);
                    }

                    raw_position_accumulate = real_t(0);
                    sw_state(SubState::PRE_LANDING);
                }
                break;
            
            case SubState::PRE_LANDING:
                odo.update();

                setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
                if(cnt >= landingpreturns * subdivide_micros){
                    landing_position_accumulate = real_t(0);
                    openloop_pole = 0;
                    odo.setElecRadOffset(real_t(0));
                    sw_state(SubState::LANDING);
                }
                break;
            case SubState::LANDING:
                odo.update();

                setCurrent(real_t(cali_current), sin(real_t(cnt % (subdivide_micros * 2)) / real_t(subdivide_micros) * PI));
                if(cnt % subdivide_micros == 0){
                    landing_position_accumulate += raw_position_accumulate;
                    elecrad_test_data[openloop_pole] = odo.getElecRad();
                    openloop_pole++;
                }

                if(cnt >= landingturns * subdivide_micros){
                    sw_state(SubState::STOP);
                }
            case SubState::STOP:
                odo.update();

                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * stop_ms)){
                    sw_state(SubState::ANALYSIS);
                }

                
                break;
            case SubState::ANALYSIS:
                // odo.runCaliAnalysis();
                // for(uint8_t i = 0; i < 50; i++){
                //     odo.cali_map[i] = mean(forward_test_data[i].second, backward_test_data[i].second);
                // }

                // for(auto & forward_err_item : forward_err){
                //     forward_err_item -= round(forward_err_item);
                // }

                // for(auto & backward_err_item : backward_err){
                //     backward_err_item -= round(backward_err_item);
                // }

                // {
                //     real_t forward_max = *std::max_element(std::begin(forward_err), std::end(forward_err));
                //     real_t forward_min = *std::min_element(std::begin(forward_err), std::end(forward_err));
                // }
                // {
                //     real_t landing_position_err = landing_position_accumulate / landingturns;
                //     constexpr int iter_times = 160;
                //     // for(uint8_t i = 0; i < iter_times; i ++){
                //     //     odo.update();
                //     //     // delayMicros
                //     //     // CALI_DEBUG(odo.getRawLapPosition());
                //     //     delayMicroseconds(20);
                //     // }
                //     for(uint8_t i = 0; i < iter_times; i ++){
                //         odo.update();
                //         CALI_DEBUG(odo.getRawLapPosition() - landing_position_err);
                //     }
                // }
                odo.update();
                // odo.fixElecRadOffset(odo.getLapPosition() - landing_position_accumulate / landingturns);

                // odo.addPostDynamicFixPosition(-odo.warp_around_zero(landing_position_accumulate / landingturns));
                // odo.fixElecRad();
                // odo.adjustZeroOffset();
                // setCurrent(real_t(align_current), real_t(PI / 2));
                // delay(100);
                // odo.update();
                // CALI_DEBUG(l-anding_position_accumulate / landingturns, odo.getElecRad());

                for(auto & item : elecrad_test_data){
                    CALI_DEBUG(item);
                }

                odo.setElecRadOffset(std::accumulate(std::begin(elecrad_test_data), std::end(elecrad_test_data), real_t(0)) / real_t(int(elecrad_test_data.size())));
                                CALI_DEBUG(odo.getElecRad());
                // setCurrent(real_t(align_current), real_t(0));
                // delay(100);
                // odo.update();
                // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());

                // setCurrent(real_t(align_current), real_t(-PI / 2));
                // delay(100);
                // odo.update();
                // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());

                // setCurrent(real_t(align_current), real_t(0));
                // delay(100);
                // odo.update();
                // CALI_DEBUG(odo.getRawLapPosition(), odo.getElecRad());
                sw_state(SubState::EXAMINE);
                break;
            case SubState::EXAMINE:
                // for(uint8_t i = 0; i < 50; i++){
                //     // logger << forward_test_data[i].first << ", " << forward_test_data[i].second << ", " << forward_err[i] << ", " << backward_test_data[i].first << ", " << backward_test_data[i].second << ", " <<  backward_err[i] << endl;
                //     CALI_DEBUG(forward_err[i], backward_err[i]);
                // }

                // for(uint8_t i = 0; i < poles; i++){
                    
                // }
                // logger.setEps(4);
                // logger.setSpace(", ");
                // for(uint8_t i = 0; i < poles; i++){
                //     CALI_DEBUG(forward_err.at(i), backward_err.at(i));
                // }

                odo.locateRelatively(real_t(0));
                setCurrent(real_t(0), real_t(0));
                sw_state(SubState::DONE);
                break;

            case SubState::DONE:
                return true;
                break;
            default:
                break;
        }
        cnt++;
    }
    return false;
}