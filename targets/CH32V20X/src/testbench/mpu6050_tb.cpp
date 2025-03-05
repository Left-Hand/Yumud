#include "tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"


using namespace ymd::drivers;

// #define UART uart2
#define UART uart1
void mpu6050_tb(OutputStream & logger, hal::I2c & i2c){
    MPU6050 mpu{i2c};

    mpu.init();
    mpu.setAccRange(MPU6050::AccRange::_2G);

    while(true){
        mpu.update();
        // auto [x,y,z] = +(+mpu.getGyr2());
        auto [x,y,z] = (+mpu.getGyr2());
        // auto [x,y,z] =/ mpu.getGyr();
        // auto [x,y,z] = mpu.getAcc();
        // const auto acc = mpu.getAcc();
        // logger.noBrackets();
        // logger.println(acc);
        logger.println(x,y,z);
        // delay(2);
    }
}

void mpu6050_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    I2cSw i2c{portA[12], portA[15]};
    i2c.init(400_KHz);
    // i2c.init();

    delay(200);
    mpu6050_tb(DEBUGGER, i2c);
}