#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Magnetometer/MMC5983/MMC5983.hpp"
#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd::drivers;

// #define UART uart2
#define UART uart2
#define SCL_GPIO hal::portB[3]
#define SDA_GPIO hal::portB[5]
static constexpr uint FS = 100;
static constexpr auto INV_FS = (1.0_q24 / FS);
// #define MAG_ACTIVATED


static void mmc5983_test(drivers::MMC5983 & imu){

    if(const auto res = imu.init();
        res.is_err()) PANIC(res.unwrap_err());

    // DEBUG_PRINTLN(do_set().unwrap());

    // while(true);
    // const auto m = millis();
    const auto base = (imu.do_magreset().unwrap() + imu.do_magset().unwrap())/2;
    // PANIC(millis() - m);
    while(true){
        // if(const auto res = imu.update();
        //     res.is_err()) PANIC(res.unwrap_err());

        // imu.read_mag().unwrap();
        // DEBUG_PRINTLN(imu.read_mag());


        imu.update().unwrap();
        const auto mag = imu.read_mag().unwrap() - base;
        DEBUG_PRINTLN(Quat_t<q24>::from_direction(mag).to_euler(), mag, atan2(mag.y, mag.x));

        // DEBUG_PRINTLN(millis(), imu.read_mag().unwrap(), imu.read_temp().unwrap());
        delay(10);
    }
    // imu.set_mode(MMC5983::Mode::Cont4).unwrap();
    // DEBUG_PRINTLN("app started");

    // Quat_t<q24> gest;
    // auto measure = [&](){
    //     imu.update().unwrap();
    //     const auto dir = imu.read_mag().unwrap();
    //     gest = gest.slerp(Quat_t<q24>::from_direction(dir), 0.05_r);
    // };

    // timer1.init(FS);
    // timer1.attach(TimerIT::Update, {0,0},[&]{
    //     measure();
    // });
    
    // while(true){
    //     // DEBUG_PRINTLN(imu.update());
    //     DEBUG_PRINTLN(millis(), gest, imu.read_mag());
    // }
}

void mmc5983_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(true);

    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{SCL_GPIO, SDA_GPIO};
    // i2c.init(400_KHz);
    i2c.init(200_KHz);
    // i2c.init();

    delay(200);


    MMC5983 imu = {i2c};

    // auto & spi = spi1;
    // spi.init(18_MHz);
    // MMC5983 imu = {SpiDrv(spi, spi.attach_next_cs(portA[15]).value())};

    mmc5983_test(imu);
}