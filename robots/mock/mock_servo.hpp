#pragma once

#include "core/math/realmath.hpp"
#include "dsp/state_vector.hpp"


namespace ymd::robots::mock{
struct MotorCmd{
    iq16 ref_x1;
    iq16 ref_x2;
};

class MotorIntf{
    virtual void set_motorcmd(const MotorCmd & cmd);
};

class MockServo final:public MotorIntf{
public:
    void set_angle(const iq16 angle){
        // DEBUG_PRINTLN("set", angle_);
        angle_ = angle;
    }

    iq16 get_angle(){
        // DEBUG_PRINTLN("get", angle_);
        return angle_;
    }

    void set_motorcmd(const MotorCmd & cmd){
        set_angle(cmd.ref_x1 * iq16(M_PI));
    }
private:
    iq16 angle_;
};

}
