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

using namespace ymd::drivers;

// #define UART uart2
#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()
#define MAG_ACTIVATED

[[maybe_unused]] static auto init_mpu6050(MPU6050 & mpu) -> Result<void, MPU6050::Error> {
    mpu.set_package(MPU6050::Package::MPU6050);
    if(const auto res = mpu.init({});
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.set_acc_fs(MPU6050::AccFs::_2G);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.enable_direct_mode(EN);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

[[maybe_unused]] static void ak8963_tb(hal::I2c & i2c){
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

[[maybe_unused]] static void mpu6050_tb(hal::I2c & i2c){
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

[[maybe_unused]] static void mpu6500_tb(hal::I2c & i2c){
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

    // ImuFusion fusion;

    // const real_t mahony_tau = 10.5_r;
    Mahony mahony{{
        .kp = 2,
        .ki = 0.3_r,
        .fs = 200
    }};

    auto & timer = hal::timer1;
    
    timer.init({
        .count_freq = hal::NearestFreq(200),
        .count_mode = hal::TimerCountMode::Up
    }, EN);

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

            const auto begin_m = clock::micros();

            // mahony.update(
            //     mpu.read_gyr().examine(), 
            //     mpu.read_acc().examine()
            //     // aku.read_mag().examine()
            // );

            
            mahony.update(
                mpu.read_gyr().examine(), 
                mpu.read_acc().examine()
            );
                
            const auto end_m = clock::micros();
            // DEBUG_PRINTLN(fusion.quat());
            // DEBUG_PRINTLN(Basis<real_t>(mahony.result()).get_euler_xyz(), end_m - begin_m);
            // DEBUG_PRINTLN(mahony.result());
            DEBUG_PRINTLN(
                mahony.rotation(), 
                // Quat<real_t>(Vec3<real_t>(0,0,1), aku.read_mag().examine().normalized()), 
                end_m - begin_m
            );
            break;
        }
        default: break;
        }
    });

    while(true);
}


void mpu6050_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    // i2c.init(400_KHz);
    i2c.init({400_KHz});
    // i2c.init();

    clock::delay(200ms);

    mpu6050_tb(i2c);
    // mpu6500_tb(i2c);
    // ak8963_tb(i2c);
}