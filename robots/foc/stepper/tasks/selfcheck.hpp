#pragma once

#include "robots/foc/motor_utils.hpp"


namespace yumud::foc{
using namespace yumud::drivers;
class SelfCheckTasker{
protected:
    scexpr int subdivide_micros = 1024;
    scexpr int check_times = 2;
    scexpr real_t minimal_motion = inv_poles / 8 * real_t(0.9);
    
    enum class SubState{
        INIT,
        TEST_MAG,
        TEST_A,
        TEST_B,
        CHECK_DIR,
        STOP,
        DONE
    };

    SubState sub_state = SubState::INIT;
    int cnt;
    Range_t<real_t> move_range;

    SVPWM & svpwm_;
    OdometerPoles & odo_;

    using ErrorCode = MotorUtils::ErrorCode;
    
    void sw_state (const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };
public:
    SelfCheckTasker(SVPWM & _svpwm, OdometerPoles & _odo):
        svpwm_(_svpwm), odo_(_odo){;}

    void reset();
    void run();
    bool done();
};

}