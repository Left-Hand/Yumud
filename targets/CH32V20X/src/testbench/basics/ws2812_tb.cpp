#include "src/testbench/tb.h"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/CommonIO/Led/WS2812/ws2812.hpp"
#include "drivers/Modem/dshot/dshot.hpp"


using namespace ymd;


#define TARG_UART hal::uart2

[[maybe_unused]] static void ws2812_tb(hal::GpioIntf & gpio){
    gpio.outpp();
    drivers::WS2812 led{gpio};
    led.init();
    while(true){
        const auto hue = 0.5_r + 0.5_r * math::sin(clock::time());
        const auto amp = 0.2_r;
        led.set_rgb(RGB<real_t>::from_hsv(hue,1,1) * amp);
        clock::delay(10ms);
    }
}


void ws2812_main(){
    auto gpio = hal::PB<1>();
    ws2812_tb(gpio);
}