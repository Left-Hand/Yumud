#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::drivers;


void ma730_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    DEBUGGER_INST.init({576_KHz});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & spi = hal::spi1;
    spi.init({9_MHz});
    auto spi_cs_gpio_ = hal::PA<15>();
    MA730 ma730{&spi, spi.allocate_cs_gpio(&spi_cs_gpio_).examine()};
    ma730.init({
        .direction = CW
    }).examine();

    while(true){
        ma730.update().examine();
        DEBUG_PRINTLN(ma730.read_lap_angle().examine());
    }
}