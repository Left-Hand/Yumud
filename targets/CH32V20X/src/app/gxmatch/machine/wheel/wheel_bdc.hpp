#include "wheel.hpp"

namespace gxm{

class WheelBdc:public Wheel{
protected:

public:
    WheelBdc(const Config & config):Wheel(config){;}
    DELETE_COPY_AND_MOVE(WheelBdc)

    void setMotorSpeed(const real_t spd) override;
    void setMotorPosition(const real_t pos) override;
    real_t getMotorSpeed() override;
    real_t getMotorPosition() override;
};


}