#include "tb.h"
#include "sys/debug/debug_inc.h"
#include "canopen/Entry.hpp"
#include "canopen/ObjectDict.hpp"

using namespace ymd;

void canopen_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    canopen::SdoObjectDict sdo_od;
    
    uint16_t data = 0;
    while(true){

        const auto m = micros();
        for(size_t i = 0; i < 1000; i++){
            // const auto write_err = sdo_od.write<uint16_t>(data+100, {0x1200, 0x01});
            const auto read_err = sdo_od.read<uint16_t>(data, {0x1200, 0x01});
        }
        
        DEBUG_PRINTLN(data, sdo_od.ename({0x1200, 0x01}), micros() - m);
        
        delay(1);
    }
}   