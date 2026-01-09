#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/VCE2755/vce2755.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::drivers;


void vce2755_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & spi = hal::spi2;

    spi.init({
        .remap = hal::SpiRemap::_0,
        .baudrate = hal::NearestFreq(18_MHz)
    });

    auto spi_cs_pin_ = hal::PB<12>();
    spi_cs_pin_.outpp();
    VCE2755 vce2755{&spi, spi.allocate_cs_pin(&spi_cs_pin_).examine()};
    vce2755.init({
        .direction = CW
    }).examine();

    while(true){
        vce2755.update().examine();
        const auto lap_angle = vce2755.read_lap_angle().examine();
        DEBUG_PRINTLN(
            lap_angle.to_turns(),
            static_cast<int32_t>(lap_angle.to_turns().to_bits() >> (32 - 18))
        );
    }
}