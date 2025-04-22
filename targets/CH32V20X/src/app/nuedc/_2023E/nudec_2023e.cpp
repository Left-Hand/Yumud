#include "src/testbench/tb.h"



#include "threads.hpp"

namespace nudec::_2023E{

class App{
public: 
    struct Config{
        ServoConfig yaw_cfg;
        ServoConfig pitch_cfg;
        GimbalPlanner::Config gimbal_cfg;
    };
};


auto & SERVO_PWMGEN_TIMER = hal::timer3;
auto & DBG_UART = hal::uart2;
// static constexpr auto CTRL_FREQ = 10_KHz;
static constexpr auto CTRL_FREQ = 50;

static constexpr auto make_cfg(){
    return App::Config{
        .yaw_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        },
        .pitch_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        },
        .gimbal_cfg = {
            .dyna_cfg = {
                .r = 2,
                .max_spd = 1_r,
                .fs = CTRL_FREQ
            },
            .kine_cfg = {
                .gimbal_base_height = 0.1_r,
                .gimbal_dist_to_screen = 1.0_r,
                .screen_width = 1.0_r,
                .screen_height = 1.0_r,
            }
        }
    };
}

}

void nuedc_2023e_main(){
    using namespace nudec::_2023E;


    DBG_UART.init(576000);
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();

    DEBUG_PRINTLN(std::setprecision(4));

    constexpr auto cfg = make_cfg();


    SERVO_PWMGEN_TIMER.init(50);
    hal::TimerOC & pwm_yaw = SERVO_PWMGEN_TIMER.oc(1);
    hal::TimerOC & pwm_pitch = SERVO_PWMGEN_TIMER.oc(2);

    pwm_yaw.init();
    pwm_pitch.init();

    pwm_yaw.set_sync();
    pwm_pitch.set_sync();

    pwm_yaw.set_polarity(false);
    pwm_pitch.set_polarity(false);

    auto servo_yaw = PwmServo::make_sg90(cfg.yaw_cfg, pwm_yaw);

    auto servo_pitch = PwmServo::make_sg90(cfg.pitch_cfg, pwm_pitch);

    auto gimbal_actuator = GimbalActuatorByLambda({
        .yaw_setter = [&servo_yaw](const MotorCmd cmd){
            servo_yaw.set_motorcmd(cmd);
        },
        .pitch_setter = [&servo_pitch](const MotorCmd cmd){
            servo_pitch.set_motorcmd(cmd);
        }
    });

    auto gimbal_planner = GimbalPlanner(cfg.gimbal_cfg, gimbal_actuator);
    SERVO_PWMGEN_TIMER.attach(TimerIT::Update, {0, 0}, [&gimbal_planner](){
        gimbal_planner.tick();
    });


    ReplThread repl_thread = ReplThread(
        DBG_UART, 
        rpc::EntryProxy{rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_thread.set_outen(true);}),
            rpc::make_function("outdis", [&](){repl_thread.set_outen(false);})
        )}
    );

    DEBUG_PRINTLN("app started");


    while(true){
        const real_t t = time();
        repl_thread.process(t);
        // DEBUG_PRINTLN(millis());
        delay(1);
    }
}