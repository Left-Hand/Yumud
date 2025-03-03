#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/VirtualIO/TCA9548A/TCA9548A.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

using namespace ymd;

void tca9548_main()
{

    uart1.init(576000);
    DEBUGGER.retarget(&uart1);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(400_KHz);

    auto tca = drivers::TCA9548A{i2c, 0x70 << 1};

    auto & vi2c = tca[0];   

    auto & act_i2c = i2c;   

    auto mpu = drivers::MPU6050{act_i2c};
    
    // mpu.init();

    DEBUGGER.noBrackets();

    while(true){
        // mpu.update();
        // DEBUG_PRINTLN(millis(), mpu.getAcc());
        DEBUG_PRINTLN(millis(), tca.verify());
        delay(20);
    }

}