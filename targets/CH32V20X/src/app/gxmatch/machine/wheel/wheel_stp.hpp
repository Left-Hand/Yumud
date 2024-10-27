#include "wheel.hpp"

namespace gxm{

class WheelStepper:public Wheel{
protected:

public:
    WheelStepper(const Config & config):Wheel(config){;}
    DELETE_COPY_AND_MOVE(WheelStepper)

    void setMotorSpeed(const real_t spd) override;
    void setMotorPosition(const real_t pos) override;
    real_t getMotorSpeed() override;
    real_t getMotorPosition() override;
};


}