#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/KTH7823/KTH7823.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::drivers;


void kth7823_main(){
    hal::uart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & spi = hal::spi1;
    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(9_MHz)
    });

    auto spi_cs_gpio = hal::PA<15>();

    kth7823::KTH7823 mag_enc{
        &spi, 
        spi.allocate_cs_pin(&spi_cs_gpio).examine()
    };

    while(true){
        mag_enc.update().examine();
        DEBUG_PRINTLN(mag_enc.read_lap_angle().examine());
        clock::delay(10ms);
    }
}