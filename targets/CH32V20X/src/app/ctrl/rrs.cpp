#include "core/clock/time.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

#include "robots/kinematics/RRS3/rrs3_kinematics.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

static constexpr uint SERVO_FREQ = 50;

void rrs_robot_main(){
    using RRS3 = typename ymd::robots::RRS_Kinematics<real_t>;
    using Gesture = typename RRS3::Gesture;
    using Config = typename RRS3::Config;

    I2cSw i2c = {portD[2], portC[12]};
    
    i2c.init(400_KHz);
    PCA9685 pca{i2c};
    pca.init();
    pca.set_frequency(SERVO_FREQ, real_t(1.09));

    MG995 servo_a{pca[0]};
    MG995 servo_b{pca[1]};
    MG995 servo_c{pca[2]};

    constexpr const Config cfg{
        .base_length = 5,
        .link_length = 5,
        .base_plate_radius = 1,
        .top_plate_radius = 1
    };

    constexpr const RRS3 rrs3{cfg};
    auto ctrl = [&]{
        const auto t = time();

        const Gesture gest{
            .orientation = Quat_t<real_t>::from_shortest_arc(
                Vector3_t<real_t>{0, 0, 1},
                Vector3_t<real_t>(0.1_r * sin(t), 0.1_r * cos(t), 1).normalized()
            ),
            .z = 5,
        };
        
        if(const auto res_opt = rrs3.inverse(gest); res_opt.is_some()){
            const auto res = res_opt.unwrap();

            servo_a.set_radian(res[0].to_absolute().j1_abs_rad);
            servo_b.set_radian(res[1].to_absolute().j1_abs_rad);
            servo_c.set_radian(res[2].to_absolute().j1_abs_rad);
        }else{
            DEBUG_PRINTLN("no solution");
        }
    };

    hal::timer1.init(SERVO_FREQ);
    hal::timer1.attach(hal::TimerIT::Update, {0,0}, ctrl);

    while(true);
}