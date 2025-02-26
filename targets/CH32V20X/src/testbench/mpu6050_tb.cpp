#include "tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"


using namespace ymd::drivers;


void mpu6050_tb(OutputStream & logger, I2c & i2c){
    MPU6050 mpu{i2c};
    mpu.init();

    while(true){
        mpu.update();
        auto [x,y,z] = mpu.getGyr();
        logger.println(x,y,z);
        delay(1);
    }
}

void mpu6050_main(){
    auto & logger = DEBUGGER_INST;
    logger.init(576_KHz);
    I2cSw i2c{portD[2], portC[12]};
    i2c.init(1_MHz);
    delay(200);
    mpu6050_tb(DEBUGGER, i2c);
}