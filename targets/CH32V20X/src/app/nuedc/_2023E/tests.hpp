#include "device.hpp"

namespace nudec::_2023E::tests{
class MockServo final:public MotorIntf{
public:
    void set_radian(const real_t rad){
        // DEBUG_PRINTLN("set", radian_);
        radian_ = rad;
    }

    real_t get_radian(){
        // DEBUG_PRINTLN("get", radian_);
        return radian_;
    }

    void set_motorcmd(const MotorCmd & cmd){
        set_radian(cmd.ref_pos * real_t(PI));
    }
private:
    real_t radian_;
};

}
