#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/AK8963/AK8963.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

// #define UART uart2
#define UART uart2
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]
#define MAG_ACTIVATED

auto init_mpu6050(MPU6050 & mpu){
    mpu.set_package(MPU6050::Package::MPU6050);
    return mpu.init() |
    mpu.set_acc_range(MPU6050::AccRange::_2G) |
    mpu.enable_direct_mode(EN);
}

void ak8963_tb(hal::I2c & i2c){
    MPU6050 mpu{i2c};
    
    if(const auto res = init_mpu6050(mpu); 
        res.is_err()) DEBUG_PRINTLN(res.unwrap_err().as<HalError>().unwrap());

    AK8963 aku{i2c};
    aku.init().unwrap();
    // aku.setAccRange(MPU6050::AccRange::_2G);

    while(true){
        aku.update().unwrap();
        clock::delay(5ms);
        DEBUG_PRINTLN_IDLE(aku.read_mag().unwrap());
    }
    while(true);
}

void mpu6050_tb(hal::I2c & i2c){
    MPU6050 mpu{i2c};

    if(const auto res = init_mpu6050(mpu); 
        res.is_err()) DEBUG_PRINTLN(res.unwrap_err().as<HalError>().unwrap());

    while(true){
        mpu.update().unwrap();
        DEBUG_PRINTLN_IDLE(
            mpu.read_acc().unwrap(),
            mpu.read_gyr().unwrap()
        );
    }
}

void mpu6500_tb(hal::I2c & i2c){
    MPU6050 mpu{i2c};

    #ifdef MAG_ACTIVATED
        AK8963 aku{i2c};
        mpu.set_package(MPU6050::Package::MPU9250);
    #else
        mpu.setPackage(MPU6050::Package::MPU6050);
    #endif

    mpu.init().unwrap();
    mpu.set_acc_range(MPU6050::AccRange::_2G).unwrap();
    mpu.enable_direct_mode(EN).unwrap();

    #ifdef MAG_ACTIVATED
        aku.init().unwrap();
    #endif

    // ImuFusion fusion;

    // const real_t mahony_tau = 10.5_r;
    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    hal::timer1.init({200});
    hal::timer1.attach(TimerIT::Update, {0,0}, [&](){
        mpu.update().examine();

        #ifdef MAG_ACTIVATED
        aku.update().unwrap();
        #endif

        // mahony.update9(
            // mpu.read_gyr().unwrap(), 
            // mpu.read_acc().unwrap(), 
            // aku.read_mag().unwrap()
        // );

        // mahony.update(
        //     mpu.read_gyr().unwrap(), 
        //     mpu.read_acc().unwrap(),
        //     aku.read_mag().unwrap()
        // );

        const auto begin_m = clock::micros();

        // mahony.update(
        //     mpu.read_gyr().unwrap(), 
        //     mpu.read_acc().unwrap()
        //     // aku.read_mag().unwrap()
        // );

        
        mahony.update(
            mpu.read_gyr().unwrap(), 
            mpu.read_acc().unwrap()
        );
            
        const auto end_m = clock::micros();
        // DEBUG_PRINTLN(fusion.quat());
        // DEBUG_PRINTLN(Basis_t<real_t>(mahony.result()).get_euler_xyz(), end_m - begin_m);
        // DEBUG_PRINTLN(mahony.result());
        DEBUG_PRINTLN(
            mahony.result(), 
            // Quat_t<real_t>(Vector3<real_t>(0,0,1), aku.read_mag().unwrap().normalized()), 
            end_m - begin_m
        );
    });

    while(true);
}


void mpu6050_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{SCL_GPIO, SDA_GPIO};
    // i2c.init(400_KHz);
    i2c.init(400_KHz);
    // i2c.init();

    clock::delay(200ms);

    mpu6050_tb(i2c);
    // mpu6500_tb(i2c);
    // ak8963_tb(i2c);
}