#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/default.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/AK8963/AK8963.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::drivers;

// #define UART usart2
#define UART hal::usart2
#define SCL_PIN hal::PB<0>()
#define SDA_PIN hal::PB<1>()
#define MAG_ACTIVATED

[[maybe_unused]] static auto init_mpu6050(MPU6050 & mpu) -> Result<void, MPU6050::Error> {
    mpu.set_package(MPU6050::Package::MPU6050);
    // const auto default_config = MPU6050::Config(Default);
    if(const auto res = mpu.init(Default);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.set_acc_fs(MPU6050::AccFs::_2G);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.enable_direct_mode(EN);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

[[maybe_unused]] static void ak8963_tb(hal::I2cBase & i2c){
    MPU6050 mpu{&i2c};
    
    init_mpu6050(mpu).examine();

    AK8963 aku{&i2c};
    aku.init().examine();
    // aku.setAccFs(MPU6050::AccFs::_2G);

    while(true){
        aku.update().examine();
        clock::delay(5ms);
        DEBUG_PRINTLN_IDLE(aku.read_mag().examine());
    }
    while(true);
}

[[maybe_unused]] static void mpu6050_tb(hal::I2cBase & i2c){
    MPU6050 mpu{&i2c};

    init_mpu6050(mpu).examine();

    while(true){
        mpu.update().examine();
        DEBUG_PRINTLN_IDLE(
            mpu.read_acc().examine(),
            mpu.read_gyr().examine()
        );
    }
}

[[maybe_unused]] static void mpu6500_tb(hal::I2cBase & i2c){
    MPU6050 mpu{&i2c};

    #ifdef MAG_ACTIVATED
        AK8963 aku{&i2c};
        mpu.set_package(MPU6050::Package::MPU9250);
    #else
        mpu.set_package(MPU6050::Package::MPU6050);
    #endif

    mpu.init({}).examine();
    mpu.set_acc_fs(MPU6050::AccFs::_2G).examine();
    mpu.enable_direct_mode(EN).examine();

    #ifdef MAG_ACTIVATED
        aku.init().examine();
    #endif

    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    auto & timer = hal::timer1;
    
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(200),
        .count_mode = hal::TimerCountMode::Up
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            mpu.update().examine();

            #ifdef MAG_ACTIVATED
            aku.update().examine();
            #endif

            // mahony.update9(
                // mpu.read_gyr().examine(), 
                // mpu.read_acc().examine(), 
                // aku.read_mag().examine()
            // );

            // mahony.update(
            //     mpu.read_gyr().examine(), 
            //     mpu.read_acc().examine(),
            //     aku.read_mag().examine()
            // );

            const auto begin_us = clock::micros();

            // mahony.update(
            //     mpu.read_gyr().examine(), 
            //     mpu.read_acc().examine()
            //     // aku.read_mag().examine()
            // );

            
            mahony.update(
                mpu.read_gyr().examine(), 
                mpu.read_acc().examine()
            );
                
            const auto end_us = clock::micros();
            // DEBUG_PRINTLN(fusion.quat());
            // DEBUG_PRINTLN(Basis<real_t>(mahony.result()).get_euler_xyz(), end_us - begin_us);
            // DEBUG_PRINTLN(mahony.result());
            DEBUG_PRINTLN(
                mahony.rotation(), 
                // Quat<real_t>(Vec3<real_t>(0,0,1), aku.read_mag().examine().normalized()), 
                end_us - begin_us
            );
            break;
        }
        default: break;
        }
    });

    timer.start();
    while(true);
}


void mpu6050_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;
    hal::I2cSw i2c{&scl_pin_, &sda_pin_};
    // i2c.init(400_KHz);
    i2c.init({
        .baudrate = hal::NearestFreq(400_KHz)
    });
    // i2c.init();

    clock::delay(200ms);

    mpu6050_tb(i2c);
    // mpu6500_tb(i2c);
    // ak8963_tb(i2c);
}