#include "tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"


using namespace ymd::drivers;


void mpu6050_tb(OutputStream & logger, hal::I2c & i2c){
    MPU6050 mpu{i2c};

    mpu.init();
    mpu.setAccRange(MPU6050::AccRange::_2G);

    while(true){
        mpu.update();
        // auto [x,y,z] = mpu.getGyr();
        // auto [x,y,z] = mpu.getAcc();
        const auto acc = mpu.getAcc();
        logger.noBrackets();
        logger.println(acc);
        delayMicroseconds(200);
    }
}

void mpu6050_main(){
    uart1.init(576_KHz);
    DEBUGGER.retarget(&uart1);
    I2cSw i2c{portA[12], portA[15]};
    i2c.init(40_KHz);

    delay(200);
    mpu6050_tb(DEBUGGER, i2c);
}