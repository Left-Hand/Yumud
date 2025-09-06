#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

void pmw3901_main(){

    uart2.init({921600, CommStrategy::Blocking});
    DEBUGGER.retarget(&uart2);
    DEBUGGER.no_brackets(EN);
    DEBUG_PRINTLN(std::setprecision(4));

    auto & spi = spi1;

    spi.init({4_MHz});

    PMW3901 pmw{&spi, spi.allocate_cs_gpio(&hal::PA<15>()).unwrap()};
    pmw.init().unwrap();

    while(true){

        const auto begin = clock::micros();
        pmw.update().unwrap();
        const auto pos = pmw.get_position();
        DEBUG_PRINTLN(pos, clock::micros() - begin);
        clock::delay(1ms);
    }
}