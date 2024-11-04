#include "tb.h"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

using namespace yumud::drivers;

using Sys::t;

void mpu6050_tb(OutputStream & logger, I2c & i2c){
    MPU6050 mpu{i2c};
    mpu.init();

    while(true){
        mpu.update();
        auto [x,y,z] = mpu.getGyro();
        logger.println(x,y,z);
        delay(1);
    }
}

void mpu6050_main(){
    auto & logger = DEBUGGER;
    logger.init(576_KHz);
    I2cSw i2c{portD[2], portC[12]};
    i2c.init(1_MHz);
    delay(200);
    mpu6050_tb(logger, i2c);
}