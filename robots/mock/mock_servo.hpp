#pragma once

#include "core/math/realmath.hpp"
#include "dsp/state_vector.hpp"


namespace ymd::robots::mock{
struct MotorCmd{
    real_t ref_pos;
    real_t ref_spd;
};

class MotorIntf{
    virtual void set_motorcmd(const MotorCmd & cmd);
};

class MockServo final:public MotorIntf{
public:
    void set_angle(const real_t angle){
        // DEBUG_PRINTLN("set", angle_);
        angle_ = angle;
    }

    real_t get_angle(){
        // DEBUG_PRINTLN("get", angle_);
        return angle_;
    }

    void set_motorcmd(const MotorCmd & cmd){
        set_angle(cmd.ref_pos * real_t(PI));
    }
private:
    real_t angle_;
};

}
