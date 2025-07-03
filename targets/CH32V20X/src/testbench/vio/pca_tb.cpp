#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

void pca_tb(OutputStream & logger){
    I2cSw i2c = {portD[2], portC[12]};

    scexpr int servo_freq = 50;
    
    i2c.init(100_KHz);
    PCA9685 pca{&i2c};
    pca.init({servo_freq, 1.09_r}).unwrap();

    MG995 servo_left{pca[0]};
    MG995 servo_right{pca[1]};

    while(true){
        servo_left.set_radian(0);
        servo_right.set_radian(0);
    }
}

void pca_main(){
    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);

    pca_tb(DEBUGGER);
}