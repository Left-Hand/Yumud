#pragma once

#include "robots/foc/motor_utils.hpp"


namespace ymd::foc{
using namespace ymd::drivers;
class SelfCheckTasker{
protected:
    static constexpr int subdivide_micros = 1024;
    static constexpr int check_times = 2;
    static constexpr real_t minimal_motion = inv_poles / 8 * real_t(0.9);
    
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
    Range2<real_t> move_range;

    digipw::SVPWM & svpwm_;
    OdometerPoles & odo_;

    using ErrorCode = MotorUtils::ErrorCode;
    
    void sw_state (const SubState new_state){
        sub_state = new_state;
        cnt = 0;
    };
public:
    SelfCheckTasker(digipw::SVPWM & _svpwm, OdometerPoles & _odo):
        svpwm_(_svpwm), odo_(_odo){;}

    void reset();
    void run();
    bool done();
};

}