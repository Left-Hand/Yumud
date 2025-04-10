#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd::drivers;

void pmw3901_main(){

    uart2.init(921600, CommStrategy::Blocking);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.no_brackets();
    DEBUG_PRINTLN(std::setprecision(4));

    auto & spi = spi1;

    spi.init(4_MHz);
    spi.bind_cs_pin(portA[15], 0);


    PMW3901 pmw{spi, 0};
    pmw.init();

    while(true){

        const auto begin = micros();
        pmw.update();
        const auto pos = pmw.get_position();
        DEBUG_PRINTLN(pos, micros() - begin);
        delay(1);
    }
}