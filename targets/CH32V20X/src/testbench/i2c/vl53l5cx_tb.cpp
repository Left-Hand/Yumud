#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/Proximeter/VL53L5CX/vl53l5cx.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

using drivers::VL53L5CX;

void vl53l5cx_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};
    i2c.init({100_KHz});

    auto red_led_gpio_ = hal::PC<13>();
    auto blue_led_gpio_ = hal::PC<14>();
    red_led_gpio_.outpp();
    blue_led_gpio_.outpp();

    auto blink_service_poller = [&]{

        red_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        blue_led_gpio_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    };


    // VL6180X vl6180{i2c, I2cSlaveAddr<7>::from_u7(0)};
    VL53L5CX vl53{&i2c};

    vl53.validate().examine();
    vl53.init().examine();


    while(true){
        blink_service_poller();
    }

}