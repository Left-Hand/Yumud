// #include "src/testbench/tb.h"

// #include "threads.hpp"
// #include "tests.hpp"

// #define USE_MOCK_SERVO


// auto & SERVO_PWMGEN_TIMER = hal::timer3;
// auto & DBG_UART = hal::uart2;
// static constexpr auto CTRL_FREQ = 50;


// namespace nudec::_2023E{

// struct AppConfig{
//     ServoConfig yaw_cfg;
//     ServoConfig pitch_cfg;
//     GimbalPlanner::Config gimbal_cfg;
// };

// class WorldFactoryBase{
// public:
//     static auto make_mock_servo(){
//         return tests::MockServo();
//     }
// };

// class WorldFactory:private WorldFactoryBase{
// public:
//     using Config = AppConfig;

//     WorldFactory(const Config & cfg_):cfg(cfg_){;}

//     const Config cfg;

//     hal::TimerOC & pwm_yaw = SERVO_PWMGEN_TIMER.oc(1);
//     hal::TimerOC & pwm_pitch = SERVO_PWMGEN_TIMER.oc(2);

//     void setup(){
//         DBG_UART.init(921600);
//         DEBUGGER.retarget(&DBG_UART);
//         DEBUGGER.no_brackets();
//         DEBUGGER.force_sync();
//         DEBUGGER.set_eps(4);

//         SERVO_PWMGEN_TIMER.init(50);

//         #ifndef USE_MOCK_SERVO

//         pwm_yaw.init({});
//         pwm_pitch.init({});

//         pwm_yaw.enable_cvr_sync();
//         pwm_pitch.enable_cvr_sync();

//         pwm_yaw.set_valid_level(HIGH);
//         pwm_pitch.set_valid_level(HIGH);
//         #endif

//         delay(20);
//     }

//     auto make_yaw_servo(){
//         #ifndef USE_MOCK_SERVO
//         return PwmServo::make_sg90(cfg.yaw_cfg, pwm_yaw);
//         #else
//         return make_mock_servo();
//         #endif
//     }

//     auto make_pitch_servo(){
//         #ifndef USE_MOCK_SERVO
//         return PwmServo::make_sg90(cfg.yaw_cfg, pwm_pitch);
//         #else
//         return make_mock_servo();
//         #endif
//     }

//     auto wtime(){
//         return time();
//     }

//     void ready(){
//         DEBUG_PRINTLN("============");
//         DEBUG_PRINTLN("app started");
//     }

//     template<typename Fn>
//     void register_servo_ctl_callback(Fn && callback){

//         SERVO_PWMGEN_TIMER.attach(TimerIT::Update, {0, 0}, std::forward<Fn>(callback));
//     }
// };

// struct Dependency{

// };
// static constexpr auto make_cfg(){
//     return AppConfig{
//         .yaw_cfg = ServoConfig{
//             .min_radian = -0.5_r,
//             .max_radian = 0.5_r
//         },
//         .pitch_cfg = ServoConfig{
//             .min_radian = -0.5_r,
//             .max_radian = 0.5_r
//         },
//         .gimbal_cfg = {
//             .dyna_cfg = {
//                 .r = 2,
//                 .max_spd = 1_r,
//                 .fs = CTRL_FREQ
//             },
//             .kine_cfg = {
//                 .gimbal_base_height = 0.1_r,
//                 .gimbal_dist_to_screen = 1.0_r,
//                 .screen_width = 1.0_r,
//                 .screen_height = 1.0_r,
//             }
//         }
//     };
// }

// }

// void nuedc_2023e_main(){
//     using namespace nudec::_2023E;

//     const auto cfg = make_cfg();

//     WorldFactory world{cfg};
//     world.setup();

//     auto servo_yaw = world.make_yaw_servo();

//     auto servo_pitch = world.make_pitch_servo();

//     auto gimbal_actuator = GimbalActuatorByLambda({
//         .yaw_setter = [&servo_yaw](const MotorCmd cmd){
//             servo_yaw.set_motorcmd(cmd);
//         },
//         .pitch_setter = [&servo_pitch](const MotorCmd cmd){
//             servo_pitch.set_motorcmd(cmd);
//         }
//     });

//     // auto gimbal_planner = GimbalPlanner(cfg.gimbal_cfg, gimbal_actuator);
//     delay(200);

//     ReplThread repl_thread = ReplThread(
//         DBG_UART, 
//         rpc::EntryProxy{rpc::make_list(
//             "list",
//             rpc::make_function("rst", [](){sys::reset();})
//             // rpc::make_function("outen", [&](){repl_thread.set_outen(true);}),
//             // rpc::make_function("outdis", [&](){repl_thread.set_outen(false);})
//             // rpc::make_function("set_rad", [&](const real_t r1, const real_t r2){
//             //     servo_pitch.set_radian(r1);
//             //     servo_yaw.set_radian(r2);
//             //     DEBUG_PRINTLN(r1, r2);
//             // }),

//             // rpc::make_function("get_rad", [&](){
//             //     DEBUG_PRINTLN(
//             //         servo_pitch.get_radian(),
//             //         servo_yaw.get_radian()
//             //     );
//             // })
//         )}
//     );

//     world.ready();

//     // world.register_servo_ctl_callback([&]{
//     //     gimbal_actuator.set_gest({0,0});
//     // });


//     while(true){
//         const real_t t = world.wtime();
//         repl_thread.process(t);
//         // DEBUG_PRINTLN(millis());
//         delay(1);
//     }
// }

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


    DBG_UART.init(921600);
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();
    DEBUGGER.set_eps(4);
    
    constexpr auto cfg = make_cfg();

    SERVO_PWMGEN_TIMER.init(50);
    hal::TimerOC & pwm_yaw = SERVO_PWMGEN_TIMER.oc(1);
    hal::TimerOC & pwm_pitch = SERVO_PWMGEN_TIMER.oc(2);

    pwm_yaw.init({});
    pwm_pitch.init({});

    pwm_yaw.enable_cvr_sync();
    pwm_pitch.enable_cvr_sync();

    pwm_yaw.set_valid_level(HIGH);
    pwm_pitch.set_valid_level(HIGH);

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