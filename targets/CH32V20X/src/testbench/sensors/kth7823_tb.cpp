#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/KTH7823/KTH7823.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::drivers;


void kth7823_main(){
    DEBUGGER_INST.init({576_KHz});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    auto & spi = hal::spi1;
    spi.init({9_MHz});

    KTH7823 kth7823{
        &spi, 
        spi.allocate_cs_gpio(&hal::PA<15>()).examine()
    };

    while(true){
        kth7823.update().examine();
        DEBUG_PRINTLN(kth7823.read_lap_angle().examine());
        clock::delay(10ms);
    }
}