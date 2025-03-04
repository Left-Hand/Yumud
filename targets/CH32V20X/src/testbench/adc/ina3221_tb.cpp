#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/INA3221/ina3221.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

using namespace ymd;

void ina3221_main()
{

    uart1.init(576000);
    DEBUGGER.retarget(&uart1);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(40_KHz);

    drivers::INA3221 ina = {i2c};

    DEBUGGER.noBrackets();

    while(true){
        // mpu.update();
        // DEBUG_PRINTLN(millis(), mpu.getAcc());
        DEBUG_PRINTLN(millis(), ina.verify());
        delay(20);
    }

}