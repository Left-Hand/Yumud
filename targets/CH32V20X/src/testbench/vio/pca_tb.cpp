#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define SCL_GPIO hal::PD<2>()
#define SDA_GPIO hal::PC<12>()

void pca_tb(OutputStream & logger){
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;

    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};

    static constexpr int servo_freq = 50;
    
    i2c.init({100_KHz});
    PCA9685 pca{&i2c};
    pca.init({
        .freq = servo_freq, 
        .trim = 1.09_r
    }).unwrap();

    MG995 servo_left{pca[0]};
    MG995 servo_right{pca[1]};

    while(true){
        servo_left.set_angle(0);
        servo_right.set_angle(0);
    }
}

void pca_main(){
    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);

    pca_tb(DEBUGGER);
}