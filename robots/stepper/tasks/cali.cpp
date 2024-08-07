#include "../robots/stepper/stepper.hpp"
#include "cali.hpp"

#include <numeric>


struct CircularTracker{
protected:
    static constexpr auto circ = inv_poles;
    real_t last_err;
public:
    void reset(){
        last_err = 0;
    }

    CircularTracker(){reset();}

    static constexpr real_t h_fmod(const real_t x, const real_t b){
        return fmod(x + b/2, b) - b/2;
    }
    static constexpr real_t calculate_err(const real_t input){
        real_t ret = h_fmod(input, circ);
        return ret;
    }
    real_t update(const real_t input){
        std::array<real_t, 3> errs;
        // 准备三个可能的选择，考虑输入值的正负和循环周期
        errs[0] = calculate_err(input);
        errs[1] = errs[0] - circ;
        errs[2] = errs[0] + circ;

        real_t min_diff = std::numeric_limits<real_t>::max();
        uint index = 0;

        for(uint i = 0; i < 3; i++){
            real_t diff = std::abs(errs[i] - last_err);
            if(diff < min_diff){
                min_diff = diff;
                index = i;
            }
        }
        // 找到这三个选择中最接近上一次输出的值，并更新最后输出
        return last_err = errs[index];
    }
};

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

    constexpr int forward_precycles = 15;
    constexpr int forward_cycles = 50;
    constexpr int backward_precycles = forward_precycles;
    constexpr int backward_cycles = forward_cycles;

    // constexpr int subdivide_micros = 256;
    constexpr int subdivide_micros = 256;
    constexpr int cogging_samples = 16;
    constexpr int align_ms = 200;

    constexpr real_t cali_current = 1.7;
    constexpr real_t align_current = 1.7;

    static SubState sub_state = SubState::DONE;
    static uint32_t cnt = 0;
    static int openloop_pole = 0;

    static std::array<real_t, poles> forward_pole_err;
    static std::array<real_t, poles> backward_pole_err;

    static std::array<real_t, cogging_samples> forward_cogging_err;
    static std::array<real_t, cogging_samples> backward_cogging_err;

    auto sw_state = [](const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };

    if(init_flag){
        sw_state(SubState::ENTRY);
        openloop_pole = 0;
        forward_pole_err.fill(0);
        backward_pole_err.fill(0);
        forward_cogging_err.fill(0);
        backward_cogging_err.fill(0);
        run_status = RunStatus::CALI;
        return RunStatus::NONE;
    }

    {
        switch(sub_state){
            case SubState::ENTRY:
                // if(autoload(true)){
                //     sw_state(SubState::EXAMINE);
                // }else{
                //     sw_state(SubState::ALIGN);
                // }
                sw_state(SubState::ALIGN);
                break;

            case SubState::ALIGN:
                setCurrent(real_t(align_current), real_t(0));
                if(cnt >= (int)((foc_freq / 1000) * align_ms)){
                    sw_state(SubState::PRE_FORWARD);
                    odo.reset();
                    odo.update();
                }
                break;

            case SubState::PRE_FORWARD:

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / subdivide_micros * TAU + PI / 2);

                if(cnt >= forward_precycles * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::FORWARD);
                }
                break;
            case SubState::FORWARD:
                odo.update();

                setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / subdivide_micros * TAU + PI/2);

                if(cnt % subdivide_micros == 0){

                    const uint cali_index = warp_mod(openloop_pole++, poles);

                    static CircularTracker tracker;
                    real_t err = tracker.update(odo.getRawLapPosition());

                    forward_pole_err[cali_index] += err / (forward_cycles / poles);
                }

                if(cnt >= forward_cycles * subdivide_micros){
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

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / subdivide_micros * TAU - PI / 2);

                if(cnt >= backward_precycles * subdivide_micros){
                    odo.update();
                    openloop_pole = odo.getRawPole();

                    sw_state(SubState::BACKWARD);
                }
                break;

            case SubState::BACKWARD:
                odo.update();

                setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / subdivide_micros * TAU - PI/2);

                if(cnt % subdivide_micros == 0){
                    const uint cali_index = warp_mod(openloop_pole--, poles);

                    static CircularTracker tracker;
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

                setCurrent(real_t(align_current), real_t(0));
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
                // odo.locateRelatively(real_t(0));
                target = 0;
                setCurrent(real_t(0), real_t(0));
                return RunStatus::EXIT;
            default:
                break;
        }
        cnt++;
    }
    return RunStatus::NONE;
}


// CaliTasker::RunStatus CaliTasker::run(const InitFlag init_flag){
//     if(init_flag){
//         reset();
//         return RunStatus::NONE;
//     }

//     switch(sub_state){
//         case SubState::ALIGN:
//             svpwm.setCurrent(real_t(align_current), real_t(0));
//             if(cnt >= (int)((foc_freq / 1000) * align_ms)){
//                 sw_state(SubState::PRE_FORWARD);
//                 odo.reset();
//                 odo.inverse();
//                 odo.update();
//             }
//             break;

//         case SubState::PRE_FORWARD:

//             svpwm.setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / subdivide_micros * TAU + PI / 2);

//             if(cnt >= forward_precycles * subdivide_micros){
//                 odo.update();
//                 openloop_pole = odo.getRawPole();

//                 sw_state(SubState::FORWARD);
//             }
//             break;
//         case SubState::FORWARD:
//             odo.update();

//             svpwm.setCurrent(real_t(cali_current), real_t(cnt % subdivide_micros) / subdivide_micros * TAU + PI/2);

//             if(cnt % subdivide_micros == 0){
//                 openloop_pole++;

//                 const uint8_t cali_index =warp_mod(openloop_pole, poles);

//                 static real_t last_err = 0;

//                 real_t err_a = fmod(odo.getRawLapPosition(), 0.02);
//                 real_t err_b = err_a - 0.02;

//                 last_err = ((ABS(err_b - last_err) > ABS(err_a - last_err))) ? err_a : err_b;

//                 forward_pole_err[cali_index] += last_err / (forward_cycles / poles);
//             }

//             if(cnt >= forward_cycles * subdivide_micros){
//                 sw_state(SubState::REALIGN);
//             }
//             break;

//         case SubState::REALIGN:
//             svpwm.setCurrent(real_t(align_current), real_t(0));
//             if(cnt >= (int)((foc_freq / 1000) * align_ms)){
//                 sw_state(SubState::PRE_BACKWARD);
//             }
//             break;

//         case SubState::PRE_BACKWARD:

//             svpwm.setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / subdivide_micros * TAU - PI / 2);

//             if(cnt >= backward_precycles * subdivide_micros){
//                 odo.update();
//                 openloop_pole = odo.getRawPole();

//                 sw_state(SubState::BACKWARD);
//             }
//             break;

//         case SubState::BACKWARD:
//             odo.update();

//             svpwm.setCurrent(real_t(cali_current), -real_t(cnt % subdivide_micros) / subdivide_micros * TAU - PI/2);

//             if(cnt % subdivide_micros == 0){
//                 openloop_pole--;

//                 const uint8_t cali_index = warp_mod(openloop_pole, poles);

//                 static real_t last_err = 0.1;

//                 real_t err_a = fmod(odo.getRawLapPosition(), 0.02);
//                 real_t err_b = err_a - 0.02;

//                 last_err = ((ABS(err_b - last_err) > ABS(err_a - last_err))) ? err_a : err_b;
//                 // last_err = err_a;

//                 backward_pole_err[cali_index] += last_err / (backward_cycles / poles);
//             }

//             if(cnt >= backward_cycles * subdivide_micros){
//                 sw_state(SubState::STOP);
//                 openloop_pole = 0;
//             }
//             break;

//         case SubState::STOP:
//             odo.update();

//             svpwm.setCurrent(real_t(align_current), real_t(0));
//             if(cnt >= (int)((foc_freq / 1000) * align_ms)){
//                 sw_state(SubState::ANALYSIS);
//             }

            
//             break;
//         case SubState::ANALYSIS:
//             {
//                 real_t forward_mean = std::accumulate(std::begin(forward_pole_err), std::end(forward_pole_err), real_t(0)) / poles;
//                 real_t backward_mean = std::accumulate(std::begin(backward_pole_err), std::end(backward_pole_err), real_t(0)) / poles;

//                 if(forward_mean < backward_mean){
//                     for(auto & item : forward_pole_err) item += inv_poles;
//                 }

//                 for(size_t i = 0; i < poles; i++){
//                     odo.map()[i] = mean(forward_pole_err[i], backward_pole_err[i]);
//                 }
//             }

//             sw_state(SubState::EXAMINE);
//             break;
//         case SubState::EXAMINE:

//             odo.locateRelatively(real_t(0));
//             svpwm.setCurrent(real_t(0), real_t(0));
//             sw_state(SubState::DONE);
//             break;

//         case SubState::DONE:
//             return RunStatus::EXIT;
//             break;
//         default:
//             break;
//     }
//     cnt++;

//     return RunStatus::NONE;
// }