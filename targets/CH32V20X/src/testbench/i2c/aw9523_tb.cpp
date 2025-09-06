#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/VirtualIO/AW9523/aw9523.hpp"

#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define DBG_UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

using drivers::AW9523;

void aw9523_main(){
    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.no_brackets();

    hal::I2cSw i2c = {&SCL_GPIO, &SDA_GPIO};
    i2c.init(200_KHz);

    AW9523 aw9523{&i2c};
    
    aw9523.init({
        .current_limit = AW9523::CurrentLimit::Low
    }).examine();

    aw9523.enable_led_mode(hal::PinMask::from_u16(0x0000))
        .examine();

    while(true){
        const auto dutycycle = (0.5_r + 0.5_r * sin(clock::time()));
        aw9523.set_led_current_dutycycle(
            hal::PinMask::from_u16(0xffff),
            dutycycle
        ).examine();
        clock::delay(5ms);
        DEBUG_PRINTLN(clock::millis(), dutycycle);
    }

}