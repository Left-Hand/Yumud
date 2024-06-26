#pragma once

#include "../constants.hpp"

class CaliTasker{
protected:
    using RunStatus = StepperEnums::RunStatus;
    using InitFlag = StepperEnums::InitFlag;

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

    static constexpr int forwardpreturns = 15;
    static constexpr int forwardturns = 100;
    static constexpr int backwardpreturns = forwardpreturns;
    static constexpr int backwardturns = forwardturns;

    static constexpr int subdivide_micros = 256;
    static constexpr int cogging_samples = 16;
    static constexpr int align_ms = 200;

    static constexpr float cali_current = 0.6;
    static constexpr float align_current = 0.6;


    SubState sub_state = SubState::DONE;
    uint32_t cnt = 0;
    int openloop_pole = 0;

    std::array<real_t, poles> forward_pole_err;
    std::array<real_t, poles> backward_pole_err;

    std::array<real_t, cogging_samples> forward_cogging_err;
    std::array<real_t, cogging_samples> backward_cogging_err;

    SVPWM2 & svpwm;
    OdometerPoles & odo;

    void sw_state(const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };
public:
    CaliTasker(SVPWM2 & _svpwm, OdometerPoles & _odo):svpwm(_svpwm), odo(_odo){};
    RunStatus run(const InitFlag init_flag);
    void reset(){
        cnt = 0;
        sub_state = SubState::ALIGN;
        openloop_pole = 0;

        forward_pole_err.fill(0);
        backward_pole_err.fill(0);
        forward_cogging_err.fill(0);
        backward_cogging_err.fill(0);
    }
};