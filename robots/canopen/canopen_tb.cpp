#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "canopen/Entry.hpp"
#include "canopen/ObjectDict.hpp"
#include "canopen/cia402.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::canopen;

void canopen_main(){
    DEBUGGER_INST.init(576000);
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    // SdoObjectDict od;
    Cia402ObjectDict od;
    
    uint32_t data = 0;

    const Didx index = {0x6061, 0x00};

    while(true){

        const auto m = clock::micros();
        // for(size_t i = 0; i < 1000; i++){
        //     od.find(index);
        // }
        auto et = od.find(index).value();

        et.write_any(data + 1);
        et.read_any(data);
        // et.(&data, index);
        DEBUG_PRINTLN(data, et.name(), et.dsize(), clock::micros() - m);
        // DEBUG_PRINTLN(data, et.name(),(index), micros() - m);
        
        clock::delay(1ms);
    }
}   