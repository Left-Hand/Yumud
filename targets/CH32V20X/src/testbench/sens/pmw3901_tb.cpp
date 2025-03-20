#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/gpio/port.hpp"

using namespace ymd::drivers;

void pmw3901_main(){

    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);

    DEBUG_PRINTLN(std::setprecision(4));

    auto & spi = spi1;


    spi.init(9_MHz);
    spi.bind_cs_pin(portD[5], 0);

    PMW3901 pmw{spi, 0};
    pmw.init();
    // pmw.update();
    
    // Vector2_t<int16_t> pos;
    while(true){

        // pos += Vector2(pmw.getPosition());
        // auto pos = Vector2(pmw.getMotion());

        auto begin = micros();
        pmw.update();
        auto pos = pmw.getPosition();
        auto [x,y] = pos;
        DEBUG_PRINTLN(x,y, micros() - begin);
        // delayMicroseconds(5000);
        delay(5);
    }
}