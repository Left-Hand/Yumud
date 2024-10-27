#include "wheel.hpp"


class Coil2Driver;
namespace gxm{

class WheelBdc:public Wheel{
protected:
    using Driver = Coil2Driver;
    Driver & driver_;

    // struct{
    real_t targ_spd;
    real_t targ_pos;
    // };

    template<arithmetic T>
    struct IncPID_t{
        
        T sum;
        T kp;
        T kd;
    };
    
public:
    WheelBdc(const Config & config, Driver & driver):Wheel(config), driver_(driver){;}
    DELETE_COPY_AND_MOVE(WheelBdc)

    void setMotorSpeed(const real_t spd) override;
    void setMotorPosition(const real_t pos) override;
    real_t getMotorSpeed() override;
    real_t getMotorPosition() override;
};


}