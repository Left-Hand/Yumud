#include "src/testbench/tb.h"

#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

#include "robots/kinematics/RRS3/rrs3_kinematics.hpp"
#include "middlewares/rpc/repl_server.hpp"
#include "algebra/transforms/euler.hpp"

#define MOCK_TEST_ALL

using namespace ymd;

using namespace ymd::drivers;

#define DBG_UART DEBUGGER_INST
static constexpr uint SERVO_FREQ = 50;
#define SCL_PIN hal::PB<0>()
#define SDA_PIN hal::PB<1>()



#ifdef MOCK_TEST_ALL
#define USE_MOCK_SERVO
#define USE_MOCK_TIME
#define USE_MOCK_OUTPUT
#else
// #define USE_MOCK_SERVO
// #define USE_MOCK_TIME
// #define USE_MOCK_OUTPUT
#endif


class MockServo{
protected:
    Angular<real_t> now_angle_;
public:
    void set_angle(const Angular<real_t> angle){
        now_angle_ = angle;
    }
    Angular<real_t> get_angle(){
        return now_angle_;
    }
};


class Environment{
public:
    hal::Gpio scl_pin = SCL_PIN;
    hal::Gpio sda_pin = SDA_PIN;
    hal::I2cSw i2c = hal::I2cSw{&scl_pin, &sda_pin};
    PCA9685 pca{&i2c};


    #ifndef USE_MOCK_SERVO

    MG995 servo_a{pca[0]};
    MG995 servo_b{pca[1]};
    MG995 servo_c{pca[2]};
    #else
    MockServo servo_a;
    MockServo servo_b;
    MockServo servo_c;
    #endif

    void setup(){
        // #ifdef USE_MOCK_SERVO

        DBG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(576_KHz),
        });

        DEBUGGER.retarget(&DBG_UART);
        // DEBUGGER.no_brackets();
        DEBUGGER.force_sync(EN);


        i2c.init({
            .baudrate = hal::NearestFreq(400_KHz)
        });

        #ifndef USE_MOCK_SERVO
        if(const auto res = [&]{
            return pca.init({.freq = SERVO_FREQ, .trim = 0.991_r});
        }(); res.is_err()) PANIC(res.unwrap_err().as<HalError>().unwrap());

        #endif

        hal::timer1.init({
            .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
            .count_freq = hal::NearestFreq(SERVO_FREQ),
            .count_mode = hal::TimerCountMode::Up
        })
                .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();
        hal::timer1.start();
    }

    template<typename Fn>
    void register_servo_ctl_callback(Fn && fn){
        auto & timer = hal::timer1;
        timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
        timer.enable_interrupt<hal::TimerIT::Update>(EN);
        timer.set_event_handler(std::forward<Fn>(fn));
    }


    void loop(){
        
    }
};

class RRS3_RobotActuator{
public:
    using RRS3_Kinematics = typename ymd::robots::kinematics::RRS_Kinematics<real_t>;
    using Gesture = typename RRS3_Kinematics::Gesture;
    using Config = typename RRS3_Kinematics::Config;

    using ServoSetter = std::function<
        void(Angular<real_t>, Angular<real_t>, Angular<real_t>)>;

    template<typename T>
    using IResult = RRS3_Kinematics::IResult<T>;

    template<typename SetterFn>
    RRS3_RobotActuator(
        const Config & cfg, 
        SetterFn && servo_setter_fn
    ): 
        rrs3_kine_{cfg},
        servo_setter_(std::forward<SetterFn>(servo_setter_fn)){;}

    void set_gest(const Gesture gest){
        
        if(const auto may_solu = rrs3_kine_.inverse(gest); may_solu.is_some()){
            const auto solu = may_solu.unwrap();

            apply_angles_to_servos({
                solu[0].to_absolute().j1_abs,
                solu[1].to_absolute().j1_abs,
                solu[2].to_absolute().j1_abs
            });
        }else{
            DEBUG_PRINTLN("no solution");
        }
    };

    constexpr void set_bias(const real_t a, const real_t b, const real_t c){
        r_bias_[0].set_radians(a);
        r_bias_[1].set_radians(b);
        r_bias_[2].set_radians(c);
    }

    void go_idle(){
        apply_angles_to_servos({0,0,0});
    }

    auto make_rpc_list(const StringView name){
        return rpc::make_list(
            name,
            rpc::make_function("gest",  
                [&](const real_t yaw, const real_t pitch, const real_t height){
                    this->set_gest(Gesture::from({
                        .yaw = yaw, .pitch = pitch, .height = height}));
                }
            ),
            rpc::make_memfunc("set_bias",  this, &RRS3_RobotActuator::set_bias),
            rpc::make_memfunc("go_idle",  this, &RRS3_RobotActuator::go_idle)
        );
    }
private:
    RRS3_Kinematics rrs3_kine_;

    std::array<Angular<real_t>,3> r_bias_ = {
        Angular<real_t>::from_radians(1.15_r),
        Angular<real_t>::from_radians(0.99_r),
        Angular<real_t>::from_radians(1.25_r)
    };

    ServoSetter servo_setter_;

    void apply_angles_to_servos(const std::array<Angular<real_t>,3> angles){
        servo_setter_(
            angles[0] + r_bias_[0], 
            angles[1] + r_bias_[1], 
            angles[2] + r_bias_[2]
        );
    }
};

void rrs3_robot_main(){

    using Config = typename RRS3_RobotActuator::Config;


    constexpr const Config cfg{
        .base_length = 0.081_r,
        .link_length = 0.128_r,
        .base_plate_radius = 0.050_r,
        .top_plate_radius = 0.08434_r
    };

    Environment env{};
    env.setup();

    auto & servo_a = env.servo_a;
    auto & servo_b = env.servo_b;
    auto & servo_c = env.servo_c;

    RRS3_RobotActuator actuator_{cfg, [&](Angular<real_t> r1, Angular<real_t> r2, Angular<real_t> r3){
        servo_a.set_angle(r1);
        servo_b.set_angle(r2);
        servo_c.set_angle(r3);
    }};

    robots::ReplServer repl_server = {
        &DBG_UART, &DBG_UART
    };

    auto rpc_list = rpc::make_list(
        "list",
        rpc::make_function("rst", [](){sys::reset();}),
        rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
        rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),
        rpc::make_function("name", [&](){
            DEBUG_PRINTLN("hello i am a robot");
        }),
        actuator_.make_rpc_list("rrs")
    );


    auto ctrl = [&]{
        const auto t = clock::seconds();
        const auto [s,c] = math::sincospu(0.7_r * t);
        
        actuator_.set_gest(
            RRS3_RobotActuator::Gesture::from({
                .yaw = Angular<real_t>::from_radians(3.0_r * s), 
                .pitch = Angular<real_t>::from_radians(3.0_r * c), 
                .height = 0.14_r
            })
        );
    };

    actuator_.go_idle();
    
    while(true){
        [[maybe_unused]]
        const real_t t = clock::seconds();
        
        repl_server.invoke(rpc_list);


        clock::delay(10ms);
        ctrl();
        // DEBUG_PRINTLN(t);
    }
}