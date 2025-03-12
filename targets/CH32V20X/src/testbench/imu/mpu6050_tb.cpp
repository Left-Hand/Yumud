#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Gyroscope/AK8963/AK8963.hpp"

#include "robots/gesture/ImuFusion.hpp"
#include "robots/gesture/mahony.hpp"

using namespace ymd::drivers;

// #define UART uart2
#define UART uart2




void ak8963_tb(hal::I2c & i2c){
    AK8963 mpu{i2c};

    mpu.init();
    // mpu.setAccRange(MPU6050::AccRange::_2G);

    while(true){
        mpu.update();
        delay(5);
        DEBUG_PRINTLN_IDLE(mpu.getMagnet().unwrap());
    }
    while(true);
}

void mpu6050_tb(hal::I2c & i2c){
    MPU6050 mpu{i2c};
    mpu.setPackage(MPU6050::Package::MPU9250);

    mpu.init();
    mpu.setAccRange(MPU6050::AccRange::_2G);
    mpu.enableDirectMode(EN);

    ak8963_tb(i2c);
    while(true){
        mpu.update();
        DEBUG_PRINTLN_IDLE(mpu.getGyr().unwrap());
    }
}

void mpu6500_tb(hal::I2c & i2c){
    MPU6050 mpu{i2c};
    mpu.setPackage(MPU6050::Package::MPU9250);

    mpu.init();
    mpu.setAccRange(MPU6050::AccRange::_2G);
    mpu.enableDirectMode(EN);

    AK8963 aku{i2c};

    aku.init().unwrap();

    // ImuFusion fusion;
    Mahony mahony{{
        .kp = 0.5_r,
        .ki = 0.000_r,
        .fs = 200
    }};

    hal::timer1.init(200);
    hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
        mpu.update();
        aku.update();

        // mahony.update9(
            // mpu.getGyr().unwrap(), 
            // mpu.getAcc().unwrap(), 
            // aku.getMagnet().unwrap()
        // );

        // mahony.update(
        //     mpu.getGyr().unwrap(), 
        //     mpu.getAcc().unwrap(),
        //     aku.getMagnet().unwrap()
        // );

        const uint32_t begin_m = micros();

        mahony.update(
            mpu.getGyr().unwrap(), 
            // mpu.getAcc().unwrap()
            aku.getMagnet().unwrap()
        );
        // fusion.update9(
        //     mpu.getGyr().unwrap(), 
        //     mpu.getAcc().unwrap(), 
        //     aku.getMagnet().unwrap()
        // );
        
        const uint32_t end_m = micros();
        // mahony.update_ai(
        //     mpu.getGyr().unwrap(), 
        //     mpu.getAcc().unwrap()
        // );

        // DEBUG_PRINTLN(fusion.quat());
        DEBUG_PRINTLN(Basis_t<real_t>(mahony.result()).get_euler_xyz(), end_m - begin_m);
        // DEBUG_PRINTLN(mahony.result());
    });

    while(true);
}


void mpu6050_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();
    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{portB[6], portB[7]};
    // i2c.init(400_KHz);
    i2c.init(400_KHz);
    // i2c.init();

    delay(200);

    // mpu6050_tb(i2c);
    mpu6500_tb(i2c);
    // ak8963_tb(i2c);
}