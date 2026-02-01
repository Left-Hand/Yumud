#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/hw_singleton.hpp"
#include "hal/bus/uart/hw_singleton.hpp"

#include "drivers/Proximeter/PMW3901/PMW3901.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd;

using namespace ymd::drivers;

void pmw3901_main(){

    hal::usart2.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576000),
    });
    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.no_brackets(EN);
    DEBUG_PRINTLN(std::setprecision(4));

    auto & spi = hal::spi1;

    spi.init({
        .remap = hal::SpiRemap::_0,
        .baudrate = hal::NearestFreq(4_MHz)
    });
    auto spi_cs_pin_ = hal::PA<15>();

    PMW3901 pmw{&spi, spi.allocate_cs_pin(&spi_cs_pin_).unwrap()};
    pmw.init().unwrap();

    while(true){

        const auto begin_us = clock::micros();
        pmw.update().unwrap();
        DEBUG_PRINTLN(clock::micros() - begin_us);
        clock::delay(1ms);
    }
}
