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

    canopen::ObjectDictionary od;

    auto et = canopen::OdEntry("a");
    auto et2 = canopen::OdEntry("abc");
    // auto et2 = canopen::SubEntry("abc");

    constexpr auto a = sizeof(et);
    // constexpr auto a = sizeof(et);

    od.insert(et, 1);
    for(size_t i = 2; i < 400; i++){
        od.insert(et2, i);
    }

    while(true){

        auto m = micros();
        for(size_t i = 0; i < 10000; i++)
            (void)od[1].unwarp().name();
        DEBUG_PRINTLN(uint32_t(micros() - m));
        DEBUG_PRINTLN(od[2].unwarp().name(), a);

        delay(1);
    }
}   