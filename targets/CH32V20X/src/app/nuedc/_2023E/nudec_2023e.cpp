#include "src/testbench/tb.h"

#include "threads.hpp"

namespace nudec::_2023E{

class App{
public: 
    struct Config{
        ServoConfig yaw_cfg;
        ServoConfig pitch_cfg;
    };
};

static constexpr auto make_cfg(){
    return App::Config{
        .yaw_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        },
        .pitch_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        }
    };
}

auto & SERVO_TIMER = hal::timer2;

void nuedc_2023e_main(){

    constexpr auto cfg = make_cfg();


    SERVO_TIMER.init(50);
    hal::TimerOC & pwm_yaw = SERVO_TIMER.oc(1);
    hal::TimerOC & pwm_pitch = SERVO_TIMER.oc(2);

    pwm_yaw.init();
    pwm_pitch.init();

    pwm_yaw.set_sync();
    pwm_pitch.set_sync();

    pwm_yaw.set_polarity(false);
    pwm_pitch.set_polarity(false);

    auto servo_yaw = PwmServo::make_sg90(cfg.yaw_cfg, pwm_yaw);

    auto servo_pitch = PwmServo::make_sg90(cfg.pitch_cfg, pwm_pitch);

    auto gimbal_actuator = GimbalActuatorByLambda({
        .yaw_setter = [&servo_yaw](const real_t radian){servo_yaw.set_radian(radian);},
        .pitch_setter = [&servo_pitch](const real_t radian){servo_pitch.set_radian(radian);}
    });
}


}