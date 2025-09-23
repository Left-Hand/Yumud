#include "src/testbench/tb.h"
#include "drivers/Wireless/Radio/CH9141/CH9141.hpp"

#include "hal/gpio/gpio_port.hpp"

#if 0
using namespace ymd;

using namespace ymd::drivers;

void ch9141_tb(IOStream & logger, Uart & uart){

    CH9141 commer{uart, hal::PC<1>(), hal::PD<3>()};
    commer.init();

    hal::PC<13>().outpp();
    while(true){
        // if(commer.available()){
        //     logger.print(commer.read());
        // }

        // if(logger.available()){
        //     commer.print(logger.read());
        // }

        // static uint32_t last_millis = millis();
        // if(millis() - last_millis >= 200){
        //     commer.println("Hello", commer.available());
        //     auto str = commer.readString();
        //     str.toUpperCase();
        //     commer.println(str);
        //     last_millis = millis();
        // }

        // if(logger.available()){
        //     clock::delay(2ms);
        //     commer.println("from l:", logger.readString());
        // }

        // hal::PC<13>() = (millis() / 200) % 2;
        
    }
}

#endif