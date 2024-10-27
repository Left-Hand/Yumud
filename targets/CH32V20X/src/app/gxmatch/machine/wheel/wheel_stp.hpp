#include "wheel.hpp"

class RemoteFOCMotor;

namespace gxm{

class WheelStepper:public Wheel{
protected:
    using Motor = RemoteFOCMotor;
    Motor & motor_;
public:
    WheelStepper(const Config & config, Motor & motor):Wheel(config), motor_(motor) {;}
    DELETE_COPY_AND_MOVE(WheelStepper)

    void setMotorSpeed(const real_t spd) override;
    void setMotorPosition(const real_t pos) override;
    real_t getMotorSpeed() override;
    real_t getMotorPosition() override;
};


}