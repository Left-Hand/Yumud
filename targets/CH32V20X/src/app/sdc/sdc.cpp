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
    X25QXX w25{SpiDrv{spi, 0}};
    std::array<uint8_t, 8> arr;
    while(true){

        // {
            arr = {1,1,4,5,1,4};
            w25.store(arr.begin(), arr.size(), 0);
            w25.load(arr.begin(), arr.size(), 0);
        // }
        // DEBUG_PRINTLN(std::oct, w25.getDeviceCapacity() >> 10, "kB", w25.getDeviceManufacturer());
        DEBUG_PRINTLN(std::oct, arr);
        delay(200);
    }
}