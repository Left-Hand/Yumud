#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"

#include "canopen/Entry.hpp"
#include "canopen/ObjectDict.hpp"
#include "canopen/cia402.hpp"

using namespace ymd;
using namespace ymd::canopen;

void canopen_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    // SdoObjectDict od;
    Cia402ObjectDict od;
    
    uint32_t data = 0;

    const Didx index = {0x6061, 0x00};

    while(true){

        const auto m = micros();
        // for(size_t i = 0; i < 1000; i++){
        //     od.find(index);
        // }
        auto et = od.find(index).value();

        et.write_any(data + 1);
        et.read_any(data);
        // et.(&data, index);
        DEBUG_PRINTLN(data, et.name(), et.dsize(), micros() - m);
        // DEBUG_PRINTLN(data, et.name(),(index), micros() - m);
        
        delay(1);
    }
}   