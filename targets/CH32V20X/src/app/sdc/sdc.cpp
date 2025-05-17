#include "../../testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "drivers/Memory/Flash/X25QXX.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd::drivers;

void w25qxx_main(){
    hal::uart2.init(576000);

    auto & spi = spi1;
    auto & w25_cs = portD[5];

    const auto spi_fd = spi.attach_next_cs(w25_cs).value();
    spi.init(36_MHz);
    X25QXX w25{hal::SpiDrv{spi, spi_fd}, 1_MB};
    std::array<uint8_t, 8> arr;
    clock::delay(20ms);
    arr = {1,1,4,5,1,4};

    scexpr size_t addr = 0;

    w25.erase(addr, 256);
    w25.store(addr,arr.begin(), arr.size());

    while(true){

        w25.load(addr,arr.begin(), arr.size());
        DEBUG_PRINTLN(std::oct, arr);
        clock::delay(200ms);
    }
}