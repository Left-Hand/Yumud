#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/conn/spi/hw_singleton.hpp"

#include "drivers/encoder/MagEnc/MA730/ma730.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::drivers;


void ma730_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    auto & spi = hal::spi1;
    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(9_MHz)
    });
    auto spi_cs_pin_ = hal::PA<15>();
    MA730 ma730{&spi, spi.allocate_cs_pin(&spi_cs_pin_).examine()};
    ma730.init({
        .direction = CW
    }).examine();

    while(true){
        ma730.update().examine();
        DEBUG_PRINTLN(ma730.read_lap_angle().examine());
    }
}