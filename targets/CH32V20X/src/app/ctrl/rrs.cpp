#include "src/testbench/tb.h"

#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

#include "robots/kinematics/RRS3/rrs3_kinematics.hpp"
#include "robots/repl/repl_thread.hpp"
#include "types/euler/euler.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

#define DBG_UART hal::uart2
static constexpr uint SERVO_FREQ = 50;
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]

// #define MOCK_TEST_ALL

#ifdef MOCK_TEST_ALL
#define USE_MOCK_SERVO
#define USE_MOCK_TIME
#define USE_MOCK_OUTPUT
#else
// #define USE_MOCK_SERVO
// #define USE_MOCK_TIME
// #define USE_MOCK_OUTPUT
#endif


struct ImpureWorldRefs{
    RadianServoIntf & servo_a;
    RadianServoIntf & servo_b;
    RadianServoIntf & servo_c;
};


class MockRadianServo final:public RadianServoIntf{
protected:
    real_t current_radian_;
public:
    void set_radian(const real_t radian){
        current_radian_ = radian;
    }
    real_t get_radian(){
        return current_radian_;
    }
};


class ImpureWorld{
public:
    ImpureWorld(const ImpureWorldRefs refs):  
        refs_(refs){;}

    void set_radian(const std::array<real_t, 3> rads){
        refs_.servo_a.set_radian(rads[0]);
        refs_.servo_b.set_radian(rads[1]);
        refs_.servo_c.set_radian(rads[2]);
    }
private:
    using Refs = ImpureWorldRefs;

    Refs refs_;
};

class HardwareFactory{
public:
    I2cSw i2c = {SCL_GPIO, SDA_GPIO};
    PCA9685 pca{i2c};


    #ifndef USE_MOCK_SERVO

    MG995 servo_a{pca[0]};
    MG995 servo_b{pca[1]};
    MG995 servo_c{pca[2]};
    #else
    MockRadianServo servo_a;
    MockRadianServo servo_b;
    MockRadianServo servo_c;
    #endif

    void setup(){
        // #ifdef USE_MOCK_SERVO

        DBG_UART.init(576000);
        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.no_brackets();
        DEBUGGER.force_sync();

        i2c.init(400_KHz);

        #ifndef USE_MOCK_SERVO
        if(const auto res = [&]{
            return pca.init({.freq = SERVO_FREQ, .trim = 0.991_r});
        }(); res.is_err()) PANIC(res.unwrap_err().as<HalError>().unwrap());

        #endif

        hal::timer1.init(SERVO_FREQ);
    }

    template<typename Fn>
    void register_servo_ctl_callback(Fn && fn){
        hal::timer1.attach(hal::TimerIT::Update, {0,0}, std::forward<Fn>(fn));
    }

    void ready(){
        DEBUG_PRINTLN("app started");
    }

    void loop(){
        
    }
};


void rrs3_robot_main(){

    using RRS3_Kinematics = typename ymd::robots::RRS_Kinematics<real_t>;
    using Gesture = typename RRS3_Kinematics::Gesture;
    using Config = typename RRS3_Kinematics::Config;


    constexpr const Config cfg{
        .base_length = 0.081_r,
        .link_length = 0.128_r,
        .base_plate_radius = 0.050_r,
        .top_plate_radius = 0.08434_r
    };

    constexpr const RRS3_Kinematics rrs3{cfg};

    HardwareFactory hw{};
    hw.setup();

    auto & servo_a = hw.servo_a;
    auto & servo_b = hw.servo_b;
    auto & servo_c = hw.servo_c;

    std::array<real_t,3> r_bias = {
        1.15_r,0.99_r,1.25_r
    };

    auto set_gest = [&](const real_t yaw, const real_t pitch, const real_t height){

        // DEBUG_PRINTLN(yaw, pitch);
        const Gesture gest{
            .orientation = Quat_t<real_t>::from_euler<EulerAnglePolicy::XYZ>({
                .x = yaw * real_t(TAU / 360), 
                .y = pitch * real_t(TAU / 360), 
                .z = 0
            }),

            .z = height,
        };

        DEBUG_PRINTLN(gest.orientation);
        
        if(const auto solu_opt = rrs3.inverse(gest); solu_opt.is_some()){
            const auto solu = solu_opt.unwrap();

            const real_t r[] = {
                solu[0].to_absolute().j1_abs_rad,
                solu[1].to_absolute().j1_abs_rad,
                solu[2].to_absolute().j1_abs_rad
            };


            servo_a.set_radian(r[0] + r_bias[0]);
            servo_b.set_radian(r[1] + r_bias[1]);
            servo_c.set_radian(r[2] + r_bias[2]);

            // const auto e = gest.orientation.to_euler();
            DEBUG_PRINTLN(
                // gest.orientation, 
                r
                // e.x, e.y, e.z
            );
        }else{
            DEBUG_PRINTLN("no solution");
        }
    };

    auto set_bias = [&](const real_t r1, const real_t r2, const real_t r3){
        r_bias[0] = r1;
        r_bias[1] = r2;
        r_bias[2] = r3;
    };

    auto set_zero = [&]{
        servo_a.set_radian(r_bias[0]);
        servo_b.set_radian(r_bias[1]);
        servo_c.set_radian(r_bias[2]);
    };

    auto get_bias = [&](){
        DEBUG_PRINTLN(r_bias);
    };

    auto out_bias = [&](const real_t r1, const real_t r2, const real_t r3){
        set_bias(r1, r2, r3);
        set_zero();
    };


    robots::ReplThread repl_thread = {
        DBG_UART, 
        rpc::EntryProxy{rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_thread.set_outen(true);}),
            rpc::make_function("outdis", [&](){repl_thread.set_outen(false);}),
            // rpc::make_function("set_gest", set_gest),
            rpc::make_function("gest", set_gest),
            rpc::make_function("set_bias", set_bias),
            rpc::make_function("get_bias", get_bias),
            rpc::make_function("set_zero", set_zero),
            rpc::make_function("out", out_bias)
        )}
    };

    auto ctrl = [&]{
        const auto t = time();
        const auto [s,c] = sincospu(0.7_r * t);
        // set_gest(5.0_r * s, 5.0_r * c, 0.14_r + 0.02_r * s);
        // set_gest(15.0_r * s, 15.0_r * c, 0.14_r);
        set_gest(3.0_r * s, 3.0_r * c, 0.14_r);
        DEBUG_PRINTLN("wh");
    };

    hw.ready();
    set_zero();
    // hw.register_servo_ctl_callback(ctrl);
    

    
    while(true){
        const real_t t = time();
        repl_thread.process(t);
        delay(10);
        ctrl();
        // DEBUG_PRINTLN(t);
    }
}