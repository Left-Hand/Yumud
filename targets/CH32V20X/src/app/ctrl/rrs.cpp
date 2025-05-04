#include "src/testbench/tb.h"

#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

#include "robots/kinematics/RRS3/rrs3_kinematics.hpp"
#include "types/euler/euler.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

#define UART hal::uart2
static constexpr uint SERVO_FREQ = 200;
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]

void rrs3_robot_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();

    using RRS3 = typename ymd::robots::RRS_Kinematics<real_t>;
    using Gesture = typename RRS3::Gesture;
    using Config = typename RRS3::Config;

    I2cSw i2c = {SCL_GPIO, SDA_GPIO};
    
    i2c.init(400_KHz);

    PCA9685 pca{i2c};

    if(const auto res = [&]{
        return pca.init({.freq = SERVO_FREQ, .trim = 0.991_r});
    }(); res.is_err()) PANIC(res.unwrap_err().as<HalError>().unwrap());
    
    MG995 servo_a{pca[0]};
    MG995 servo_b{pca[1]};
    MG995 servo_c{pca[2]};

    constexpr const Config cfg{
        .base_length = 0.081_r,
        .link_length = 0.128_r,
        .base_plate_radius = 0.050_r,
        .top_plate_radius = 0.08434_r
    };

    constexpr const RRS3 rrs3{cfg};
    auto ctrl = [&]{
        const auto t = time();

        const Gesture gest{
            // .orientation = Quat_t<real_t>::from_shortest_arc(
            //     Vector3_t<real_t>{0, 0, 1.0_r},
            //     Vector3_t<real_t>(0.5_r * sin(t), 0.5_r * cos(2*t), 1).normalized()
            // ),

            .orientation = Quat_t<real_t>::from_euler<EulerAnglePolicy::XYZ>({
                .x = 0.6_r * sinpu(t), .y = 0.6_r * cospu(t), .z = 0
            }),

            .z = 0.15_r,
        };
        
        if(const auto solu_opt = rrs3.inverse(gest); solu_opt.is_some()){
            const auto solu = solu_opt.unwrap();

            servo_a.set_radian(solu[0].to_absolute().j1_abs_rad + 0.7_r);
            servo_b.set_radian(solu[1].to_absolute().j1_abs_rad + 0.7_r);
            servo_c.set_radian(solu[2].to_absolute().j1_abs_rad + 0.7_r);

            const auto e = gest.orientation.to_euler();
            DEBUG_PRINTLN(
                pca.dump_cvr(0,1,2), 
                gest.orientation, 
                e.x, e.y, e.z,
                sizeof(PCA9685)
            );
        }else{
            DEBUG_PRINTLN("no solution");
        }


    };

    hal::timer1.init(SERVO_FREQ);
    hal::timer1.attach(hal::TimerIT::Update, {0,0}, ctrl);

    DEBUG_PRINTLN("app started");
    while(true){
        delay(1000);
    }
}