#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/VirtualIO/TCA9548A/TCA9548A.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

#define UART uart2

void tca9548_main()
{

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    DEBUGGER.noBrackets();
    
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(400_KHz);

    auto tca = drivers::TCA9548A{i2c, 0x70 << 1};

    auto & vi2c = tca[0];   

    // auto & act_i2c = i2c;   
    auto & act_i2c = vi2c;   

    auto mpu = drivers::MPU6050{act_i2c};
    
    !+mpu.init();


    while(true){
        !+mpu.update();
        DEBUG_PRINTLN(millis(), mpu.getAcc().unwrap());
        DEBUG_PRINTLN(millis(), tca.verify());
        delay(20);
    }

}