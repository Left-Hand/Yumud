#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Magnetometer/AK09911C/AK09911C.hpp"

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


static void ak09911c_test(drivers::AK09911C & aku){

    aku.init().unwrap();
    aku.set_mode(AK09911C::Mode::Cont4).unwrap();
    DEBUG_PRINTLN("app started");

    Quat_t<q24> gest;
    auto measure = [&](){
        aku.update().unwrap();
        const auto dir = aku.read_mag().unwrap();
        gest = gest.slerp(Quat_t<q24>::from_direction(dir), 0.05_r);
    };

    timer1.init(FS);
    timer1.attach(TimerIT::Update, {0,0},[&]{
        measure();
    });
    
    while(true){
        // DEBUG_PRINTLN(aku.update());
        DEBUG_PRINTLN(millis(), gest, aku.read_mag());
    }
}

void ak09911c_main(){
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


    AK09911C aku = {i2c};

    // auto & spi = spi1;
    // spi.init(18_MHz);
    // AK09911C aku = {SpiDrv(spi, spi.attach_next_cs(portA[15]).value())};

    ak09911c_test(aku);
}