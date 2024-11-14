#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"


namespace gxm{
class JointCl:public Joint{
public:
    struct Config{
        real_t max_rad_delta;
    };

protected:
    const Config & config_;

    using Motor = ymd::drivers::RadianServo;
    Motor & servo_;
    real_t last_radian_;
    real_t expect_radian_;

public:
    JointCl(const Config & config, Motor & servo):config_(config), servo_(servo){}
    DELETE_COPY_AND_MOVE(JointCl)
    
    bool reached() override;
    void setRadian(const real_t dist) override; 
    real_t getRadian(){return std::bit_cast<real_t>(last_radian_);}
    void tick() override; 
};

}