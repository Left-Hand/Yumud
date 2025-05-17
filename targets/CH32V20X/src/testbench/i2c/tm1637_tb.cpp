#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"

using namespace ymd;
// using namespace ymd::hal;
using namespace ymd::drivers;

#define UART hal::uart2

static void tm1637_tb(){
    TM1637 tm1637{hal::portB[0], hal::portB[1]};

    while(true){
        const auto res = 
            tm1637.set(0, SegDisplayer::digit_to_seg(   clock::millis().count() / 1000))
            | tm1637.set(1, SegDisplayer::digit_to_seg( clock::millis().count() / 100))
            | tm1637.set(2, SegDisplayer::digit_to_seg( clock::millis().count() / 10))
            | tm1637.set(3, SegDisplayer::digit_to_seg( clock::millis().count() % 10))
            | tm1637.flush()
        ;
        if(res.is_err()) PANIC();
        DEBUG_PRINTLN(clock::millis());
        clock::delay(20ms);
    }
}

void tm1637_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();

    tm1637_tb();
}