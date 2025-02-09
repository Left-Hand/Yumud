#include "tb.h"
#include "sys/debug/debug_inc.h"
#include "canopen/Entry.hpp"
#include "canopen/ObjectDict.hpp"
#include "canopen/cia402.hpp"

using namespace ymd;
using namespace ymd::canopen;

void canopen_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    // SdoObjectDict od;
    Cia402ObjectDict od;
    
    uint16_t data = 0;

    const Didx index = {
        0x2000, 0
    };

    while(true){

        const auto m = micros();
        // for(size_t i = 0; i < 1000; i++){
        //     od.find(index);
        // }
        auto et = od.find(index).value();

        et.write_any(data + 1);
        et.read_any(data);
        // et.(&data, index);
        DEBUG_PRINTLN(data, et.name(), micros() - m);
        // DEBUG_PRINTLN(data, et.name(),(index), micros() - m);
        
        delay(1);
    }
}   