#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/port.hpp"

using namespace ymd::drivers;

void pca_tb(OutputStream & logger){
    I2cSw i2c = {portD[2], portC[12]};

    scexpr int servo_freq = 50;
    
    i2c.init(100_KHz);
    PCA9685 pca{i2c};
    pca.init();
    pca.setFrequency(servo_freq, real_t(1.09));

    MG995 servo_left{pca[0]};
    MG995 servo_right{pca[1]};

    while(true){
        servo_left.setRadian(0);
        servo_right.setRadian(0);
    }
}

void pca_main(){
    DEBUGGER_INST.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&DEBUGGER_INST);

    pca_tb(DEBUGGER);
}