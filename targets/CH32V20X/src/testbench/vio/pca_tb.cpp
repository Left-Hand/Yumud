#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/conn/i2c/soft/soft_i2c.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/ioexpand/PCA9685/pca9685.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define SCL_PIN hal::PD<2>()
#define SDA_PIN hal::PC<12>()


void pca_tb(OutputStream & logger){
    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;

    hal::SoftI2c i2c{scl_pin_, sda_pin_};

    static constexpr int servo_freq = 50;
    
    i2c.init({hal::NearestFreq(100_KHz)});
    PCA9685 pca{&i2c};
    pca.init({
        .freq = servo_freq, 
        .trim = 1.09_iq16
    }).unwrap();

    // auto pwm1 = PCA9685Channel(pca, 0_nth);
    // auto pwm2 = PCA9685Channel(pca, 1_nth);
    // MG995 servo_left{pwm1};
    // MG995 servo_right{pwm2};

    while(true){
        // servo_left.set_angle(0);
        // servo_right.set_angle(0);
    }
}

void pca_main(){
    DEBUGGER_INST.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz), 
    });
    DEBUGGER.retarget(&DEBUGGER_INST);

    pca_tb(DEBUGGER);
}