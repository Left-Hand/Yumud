#include <atomic>
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include <barrier>

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/prelude/keycode.hpp"
#include "drivers/HID/prelude/axis_input.hpp"
#include "drivers/HID/prelude/segcode.hpp"
#include "drivers/HID/prelude/button_input.hpp"
#include "drivers/HID/FT6336/FT6336.hpp"

#include "core/debug/debug.hpp"

using namespace ymd;
using drivers::FT6336U;

#define UART hal::uart2
// #define UART hal::uart6
#define SCL_GPIO hal::PB<3>()
#define SDA_GPIO hal::PB<5>()

void ft6336_main(){
    // UART.init({576_KHz});
    UART.init({
        .baudrate = 576000,
        .tx_strategy = CommStrategy::Dma
    });
    DEBUGGER.retarget(&UART);

    
    auto & led = hal::PA<15>();
    led.outpp();

    // while(true){
    //     DEBUG_PRINTLN(clock::millis());
    //     clock::delay(30ms);
    //     led.toggle();
    // }

    // test_result();
    hal::I2cSw i2c{&SCL_GPIO, &SDA_GPIO};
    i2c.init(200_KHz);

    clock::delay(1ms);
    clock::delay(100ms);

    FT6336U ft6336{&i2c};
    ft6336.init().examine();

    while(true){
        DEBUG_PRINTLN(
            ft6336.get_touch_number().examine(),
            ft6336.get_touch1_x().examine(),
            ft6336.get_touch1_y().examine()
        );

        clock::delay(5ms);
    }
}