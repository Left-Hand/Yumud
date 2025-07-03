#include "core/utils/cpp_stl/mdspan/mdspan.hpp"
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
#include "drivers/HID/ST1615/ST1615.hpp"

#include "core/debug/debug.hpp"

using namespace ymd;
using drivers::ST1615;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

void st1615_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.force_sync();
    
    // test_result();
    hal::I2cSw i2c{&SCL_GPIO, &SDA_GPIO};
    i2c.init(100_KHz);

    ST1615 st1615{&i2c};
    st1615.init().examine();

    while(true){
        st1615.get_sensor_count().examine();
        clock::delay(5ms);
    }
}