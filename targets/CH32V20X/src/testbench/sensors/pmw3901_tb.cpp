#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"

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

        const auto begin = clock::micros();
        pmw.update().unwrap();
        const auto pos = pmw.get_position();
        DEBUG_PRINTLN(pos, clock::micros() - begin);
        clock::delay(1ms);
    }
}