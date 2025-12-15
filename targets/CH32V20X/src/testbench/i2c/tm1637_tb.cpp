#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"

using namespace ymd;
// 
using namespace ymd::drivers;

#define UART hal::uart2

static void tm1637_tb(){
    auto scl_pin_ = hal::PB<0>();
    auto sda_pin_ = hal::PB<1>();
    TM1637 tm1637{scl_pin_, sda_pin_};

    while(true){
        tm1637.set( 0, SegDisplayer::digit_to_seg( clock::millis().count() / 1000))
            .then([&]{return tm1637.set(1, SegDisplayer::digit_to_seg( clock::millis().count() / 100));})
            .then([&]{return tm1637.set(2, SegDisplayer::digit_to_seg( clock::millis().count() / 10));})
            .then([&]{return tm1637.set(3, SegDisplayer::digit_to_seg( clock::millis().count() % 10));})
            .then([&]{return tm1637.flush();})
            .examine();

        DEBUG_PRINTLN(clock::millis());
        clock::delay(20ms);
    }
}

void tm1637_main(){
    DEBUGGER_INST.init({
        hal::UART2_REMAP_PA2_PA3,
        576000
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    // DEBUGGER.no_brackets();

    tm1637_tb();
}