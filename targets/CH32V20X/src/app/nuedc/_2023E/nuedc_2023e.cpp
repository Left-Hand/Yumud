#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"

#include "service.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"




auto & SERVO_PWMGEN_TIMER = hal::timer3;
auto & DBG_UART = hal::uart2;
static constexpr auto CTRL_FREQ = 50;



using namespace ymd;

using namespace ymd::robots;

#define USE_MOCK_SERVO

namespace nuedc::_2023E{

struct AppConfig{
    ServoConfig yaw_cfg;
    ServoConfig pitch_cfg;
    GimbalPlanner::Config gimbal_planner_cfg;
};




struct Environment final{
public:
    using Config = AppConfig;


    Environment(const Config & cfg_):cfg(cfg_){;}

    const Config cfg;

    hal::TimerOC & pwm_yaw = SERVO_PWMGEN_TIMER.oc<1>();
    hal::TimerOC & pwm_pitch = SERVO_PWMGEN_TIMER.oc<2>();

    void setup(){


        SERVO_PWMGEN_TIMER.init({
            .freq = 50
        }, EN);

        #ifndef USE_MOCK_SERVO

        pwm_yaw.init({});
        pwm_pitch.init({});

        pwm_yaw.enable_cvr_sync();
        pwm_pitch.enable_cvr_sync();

        pwm_yaw.set_valid_level(HIGH);
        pwm_pitch.set_valid_level(HIGH);
        #endif

    }


    auto time(){
        return clock::time();
    }

    void ready(){
        DEBUG_PRINTLN("============");
        DEBUG_PRINTLN("app started");
    }

    template<typename Fn>
    void register_servo_ctl_callback(Fn && callback){

        SERVO_PWMGEN_TIMER.attach(hal::TimerIT::Update, {0, 0}, std::forward<Fn>(callback));
    }
private:

};


struct Factory{
    static mock::MockServo make_mock_servo(){
        return mock::MockServo();
    }

    static auto make_yaw_servo(){
        #ifndef USE_MOCK_SERVO
        return PwmServo::make_sg90(cfg.yaw_cfg, pwm_yaw);
        #else
        return make_mock_servo();
        #endif
    }

    static auto make_pitch_servo(){
        #ifndef USE_MOCK_SERVO
        return PwmServo::make_sg90(cfg.yaw_cfg, pwm_pitch);
        #else
        return make_mock_servo();
        #endif
    }

static constexpr auto make_cfg(){
    return AppConfig{
        .yaw_cfg = ServoConfig{
            .min_angle = -0.5_r,
            .max_angle = 0.5_r
        },
        .pitch_cfg = ServoConfig{
            .min_angle = -0.5_r,
            .max_angle = 0.5_r
        },
        .gimbal_planner_cfg = {
            .dyna_cfg = {
                .kp = 2.0_r,
                .kd = 2.0_r,
                .max_spd = 1_r,
                .max_acc = 1_r,
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
};

}



void nuedc_2023e_main(){
    using namespace nuedc::_2023E;
    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.force_sync(EN);
    DEBUGGER.set_eps(4);

    const auto cfg = Factory::make_cfg();

    Environment env{cfg};
    env.setup();

    auto servo_yaw = Factory::make_yaw_servo();

    auto servo_pitch = Factory::make_pitch_servo();


    auto gimbal_actuator = GimbalActuatorByLambda({
        .yaw_setter = [&servo_yaw](const MotorCmd cmd){
            servo_yaw.set_motorcmd(cmd);
        },
        .pitch_setter = [&servo_pitch](const MotorCmd cmd){
            servo_pitch.set_motorcmd(cmd);
        }
    });

    [[maybe_unused]]
    auto gimbal_planner = GimbalPlanner(cfg.gimbal_planner_cfg, gimbal_actuator);

    robots::ReplServer repl_server = {
        &DBG_UART, &DBG_UART
    };

    auto rpc_list =
        rpc::make_list( "list",
        rpc::make_function("rst", [](){sys::reset();}),
        rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
        rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
        rpc::make_function("set_rad", [&](const real_t r1, const real_t r2){
            servo_pitch.set_angle(r1);
            servo_yaw.set_angle(r2);
            DEBUG_PRINTLN(r1, r2);
        }),

        rpc::make_function("get_rad", [&](){
            DEBUG_PRINTLN(
                servo_pitch.get_angle(),
                servo_yaw.get_angle()
            );
        })
    );

    env.ready();

    // env.register_servo_ctl_callback([&]{
    //     gimbal_actuator.set_gest({0,0});
    // });
    while(true);
}
