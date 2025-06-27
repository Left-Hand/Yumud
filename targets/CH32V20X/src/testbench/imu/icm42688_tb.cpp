#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Axis6/ICM42688/icm42688.hpp"
#include "robots/gesture/mahony.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

// #define UART uart2
#define UART uart2
#define SCL_GPIO hal::portB[3]
#define SDA_GPIO hal::portB[5]
static constexpr uint ISR_FREQ = 500;
static constexpr auto INV_FS = (1.0_q24 / ISR_FREQ);
// #define MAG_ACTIVATED



[[maybe_unused]]
static void icm42688_tb(ICM42688 & imu){

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
    //     clock::delay(1ms);
    // }
    DEBUG_PRINTLN("init started");


    imu.init().unwrap();

    q24 z = 0;
    Microseconds exe = 0us;

    const real_t tau = 1.3_r;
    Mahony mahony{{
        .kp = 2 * tau,
        .ki = tau * tau,
        // .ki = 0,
        .fs = ISR_FREQ
    }};

    timer1.init({ISR_FREQ});
    timer1.attach(TimerIT::Update, {0,0},[&]{
        const auto u0 = clock::micros();
        imu.update().unwrap();
        z = z + INV_FS * imu.read_gyr().unwrap().z;
        // mahony.update(imu.read_gyr().unwrap(), imu.read_acc().unwrap());
        // mahony.update(imu.read_gyr().unwrap(), imu.read_acc().unwrap());
        mahony.myupdate_v2(imu.read_gyr().unwrap(), imu.read_acc().unwrap());
        exe = clock::micros() - u0;
    });

    while(true){
        // const auto u0 = clock::micros();
        // imu.update().unwrap();
        // const auto u1 = clock::micros();
        // const auto acc = imu.read_acc().unwrap();
        // const auto gyr = imu.read_gyr().unwrap();
        // const auto gest = Quat<real_t>::from_shortest_arc(
        //     acc.normalized(),
        //     {0,0,1}
        // );

        const auto gest = mahony.result();
        // const auto euler = gest.to_euler();
        DEBUG_PRINTLN(
            0
            ,gest
            // ,euler
            // ,acc
        //     ,gyr
        //     // ,imu.read_gyr().unwrap()
        //     // ,z, exe
        );
        clock::delay(1ms);
        // DEBUG_PRINTLN(mahony.result().normalized(), z, exe);
        // DEBUG_PRINTLN(mahony.result().to_euler(), z, exe);
        // DEBUG_PRINTLN(z);
        
        // DEBUG_PRINTLN(millis());
    }
}

void icm42688_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{SCL_GPIO, SDA_GPIO};
    // i2c.init(400_KHz);
    i2c.init(4000_KHz);
    // i2c.init();

    clock::delay(200ms);


    // ICM42688 imu = {i2c};

    auto & spi = spi1;
    spi.init({18_MHz});
    ICM42688 imu = {SpiDrv(spi, spi.attach_next_cs(portA[15]).value())};

    icm42688_tb(imu);
}