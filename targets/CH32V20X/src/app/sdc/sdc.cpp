#include "../../testbench/tb.h"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/spi/spisw.hpp"

#include "drivers/Memory/Flash/X25QXX.hpp"

using namespace yumud::drivers;

void w25qxx_main(){
    DEBUGGER_INST.init(576000);

    auto & spi = spi1;
    auto & w25_cs = portD[5];

    spi.bindCsPin(w25_cs, 0);
    spi.init(36_MHz);
    X25QXX w25{SpiDrv{spi, 0}, 1_MB};
    std::array<uint8_t, 8> arr;
    delay(20);
    arr = {1,1,4,5,1,4};

    scexpr size_t addr = 0;

    w25.erase(addr, 256);
    w25.store(addr,arr.begin(), arr.size());

    while(true){

        w25.load(addr,arr.begin(), arr.size());
        DEBUG_PRINTLN(std::oct, arr);
        delay(200);
    }
}