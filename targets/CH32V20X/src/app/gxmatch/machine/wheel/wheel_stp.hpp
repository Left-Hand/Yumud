#include "wheel.hpp"


namespace ymd::foc{
class FOCMotorConcept;
}

namespace gxm{

class WheelStepper:public Wheel{
public:
    struct Config:public Wheel::Config{

    };
protected:
    using Motor = ymd::foc::FOCMotorConcept;
    const Config & config_;
public:
    WheelStepper(const Config & config, Motor & motor):
        Wheel(config, motor), 
        config_(config){;}

    DELETE_COPY_AND_MOVE(WheelStepper)

    void setMotorSpeed(const real_t spd) override;
    void setMotorPosition(const real_t pos) override;
    real_t getMotorSpeed() override;
    real_t getMotorPosition() override;
};


}