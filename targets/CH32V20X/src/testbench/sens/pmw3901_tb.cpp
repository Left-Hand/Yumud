#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"


using namespace ymd::drivers;

void pmw3901_main(){

    DEBUGGER_INST.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    DEBUG_PRINTLN(std::setprecision(4));

    auto & spi = spi1;


    spi.init(9_MHz);
    spi.bindCsPin(portD[5], 0);

    PMW3901 pmw{spi, 0};
    pmw.init();
    // pmw.update();
    
    // Vector2_t<int16_t> pos;
    while(true){

        // pos += Vector2(pmw.getPosition());
        // auto pos = Vector2(pmw.getMotion());

        auto begin = micros();
        pmw.update();
        auto pos = Vector2(pmw.getPosition());
        auto [x,y] = pos;
        DEBUG_PRINTLN(x,y, micros() - begin);
        // delayMicroseconds(5000);
        delay(5);
    }
}