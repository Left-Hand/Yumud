#include "src/testbench/tb.h"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/CommonIO/Led/WS2812/ws2812.hpp"
#include "drivers/Modem/dshot/dshot.hpp"



using namespace ymd::hal;

#define TARG_UART hal::uart2



[[maybe_unused]] static void ws2812_tb(hal::GpioIntf & gpio){
    gpio.outpp();
    drivers::WS2812 led{gpio};
    led.init();
    while(true){
        led = Color_t<real_t>::from_hsv(0.5_r + 0.5_r * sin(time()),1,1,0.2_r);
        delay(10);
    }
}


void ws2812_main(){
    ws2812_tb(hal::portB[1]);
}