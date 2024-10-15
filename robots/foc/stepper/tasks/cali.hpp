#pragma once

#include "robots/foc/motor_utils.hpp"

class CaliTasker{
protected:
    using RunStatus = MotorUtils::RunStatus;
    using InitFlag = MotorUtils::InitFlag;

    enum class SubState{
        ENTRY,
        ALIGN,
        INIT,
        PRE_FORWARD,
        FORWARD,
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

    struct CircularTracker{
    protected:
        scexpr auto circ = inv_poles;
        real_t last_err = 0;
    public:
        CircularTracker(){;}
        
        void reset(){
            last_err = 0;
        }

        scexpr real_t h_fmod(const real_t x, const real_t b){
            return fmod(x + b/2, b) - b/2;
        }
        scexpr real_t calculate_err(const real_t input){
            real_t ret = h_fmod(input, circ);
            return ret;
        }
        real_t update(const real_t input){
            std::array<real_t, 3> errs;

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

            return last_err = errs[index];
        }
    };

    scexpr int forward_precycles = 15;
    scexpr int forward_cycles = 100;
    scexpr int backward_precycles = forward_precycles;
    scexpr int backward_cycles = forward_cycles;

    scexpr int subdivide_micros = 256 * foc_freq / 32768;
    scexpr int cogging_samples = 16;
    scexpr int align_ms = 200;

    scexpr real_t cali_current = real_t(0.6);
    scexpr real_t align_current = real_t(0.6);

    SubState sub_state = SubState::DONE;
    CircularTracker tracker;

    uint cnt = 0;
    int openloop_pole = 0;

    std::array<real_t, poles> forward_pole_err;
    std::array<real_t, poles> backward_pole_err;

    std::array<real_t, cogging_samples> forward_cogging_err;
    std::array<real_t, cogging_samples> backward_cogging_err;

    SVPWM & svpwm;
    OdometerPoles & odo;

    void sw_state(const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };
public:
    CaliTasker(SVPWM & _svpwm, OdometerPoles & _odo):svpwm(_svpwm), odo(_odo){};

    void run();
    bool done();
    void reset();
};