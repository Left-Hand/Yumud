#include "tb.h"
#include "../drivers/Wireless/Radio/CH9141/CH9141.hpp"

void ch9141_tb(IOStream & logger, Uart & uart){

    CH9141 commer{uart, portC[1], portD[3]};
    commer.init();

    portC[13].outpp();
    while(true){
        // if(commer.available()){
        //     logger.print(commer.read());
        // }

        // if(logger.available()){
        //     commer.print(logger.read());
        // }

        static uint32_t last_millis = millis();
        if(millis() - last_millis >= 200){
            commer.println("Hello", commer.available());
            auto str = commer.readString();
            str.toUpperCase();
            commer.println(str);
            last_millis = millis();
        }

        if(logger.available()){
            delay(2);
            commer.println("from l:", logger.readString());
        }

        portC[13] = (millis() / 200) % 2;
        
    }
}