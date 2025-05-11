#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/AK8963/AK8963.hpp"
#include "drivers/IMU/Axis6/ICM42688/icm42688.hpp"

#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd::drivers;

// #define UART uart2
#define UART uart2
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]
static constexpr uint FS = 1000;
static constexpr auto INV_FS = (1.0_q24 / FS);
// #define MAG_ACTIVATED

[[maybe_unused]]
static void icm42688_i2c_tb(hal::I2c & i2c){
    ICM42688 imu{i2c};
    imu.init().expect("icm42688 init failed");

    q24 z = 0;
    timer1.init(1000);
    timer1.attach(TimerIT::Update, {0,0},[&]{
        imu.update().unwrap();
        // const auto u1 = micros();
        z = z + INV_FS * imu.get_gyr().unwrap().z;
    });

    while(true){
        // const auto u0 = micros();
        // imu.update().unwrap();
        // const auto u1 = micros();
        DEBUG_PRINTLN(imu.get_acc().unwrap(), imu.get_gyr().unwrap(), z);
        // DEBUG_PRINTLN(z);
        
        // DEBUG_PRINTLN(millis());
    }
}

[[maybe_unused]]
static void icm42688_spi_tb(hal::SpiDrv && drv){
    ICM42688 imu{std::move(drv)};
    // while(true){
    //     auto test_fn = [&]{
    //         return drv.write_single<uint8_t>(uint8_t(0), CONT)
    //         | drv.write_single<uint8_t>(uint8_t(0));
    //     };

    //     if(const auto res = test_fn();  
    //         res.is_err()) DEBUG_PRINTLN(res.unwrap_err()); 
    //     else{
    //         DEBUG_PRINTLN("ok");
    //     }
    //     delay(1);
    // }
    DEBUG_PRINTLN("init started");

    imu.init().unwrap();

    q24 z = 0;
    uint32_t exe = 0;
    timer1.init(1000);
    timer1.attach(TimerIT::Update, {0,0},[&]{
        const auto u0 = micros();
        imu.update().unwrap();
        exe = micros() - u0;
        z = z + INV_FS * imu.get_gyr().unwrap().z;
    });

    while(true){
        // const auto u0 = micros();
        // imu.update().unwrap();
        // const auto u1 = micros();
        DEBUG_PRINTLN(imu.get_acc().unwrap(), imu.get_gyr().unwrap(), z, exe);
        // DEBUG_PRINTLN(z);
        
        // DEBUG_PRINTLN(millis());
    }
}

void icm42688_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync(true);

    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{SCL_GPIO, SDA_GPIO};
    // i2c.init(400_KHz);
    i2c.init(4000_KHz);
    // i2c.init();

    delay(200);

    // icm42688_i2c_tb(i2c);

    auto & spi = spi1;
    spi.init(18_MHz);
    icm42688_spi_tb(SpiDrv(spi, spi.attach_next_cs(portA[15]).value()));
    // mpu6500_tb(i2c);
    // ak8963_tb(i2c);
}