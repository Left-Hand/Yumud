#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/INA3221/ina3221.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

using namespace ymd;
using drivers::INA3221;

#define UART uart1

void ina3221_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    DEBUGGER.noBrackets();

    
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(400_KHz);

    INA3221 ina = {i2c};

    ina.init();



    while(true){
        ina.update(1);
        DEBUG_PRINTLN(ina.getBusVolt(1), ina.getShuntVolt(1));
        // mpu.update();
        // DEBUG_PRINTLN(millis(), ina.verify());
        delay(2);
    }

}