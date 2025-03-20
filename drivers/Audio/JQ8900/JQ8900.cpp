#include "JQ8900.hpp"
#include "hal/gpio/gpio_intf.hpp"

using namespace ymd::drivers;

void JQ8900::send(const uint8_t data){
    ser = true;
    delayMicroseconds(1000);
    ser = false;
    delayMicroseconds(3200);

    for(size_t i = 0; i < 8; i++){
        if((1 << i) & data){
            ser = true;
            delayMicroseconds(600);
            ser = false;
            delayMicroseconds(200);
        }else{
            ser = true;
            delayMicroseconds(200);
            ser = false;
            delayMicroseconds(600);  
        }
    }
    ser = true;
}

void JQ8900::sendNum(const int value){
    if(value > 10){
        send(value / 10);
        send(value % 10);
    }else{
        send(value);
    }
}

void JQ8900::sendLine(const uint8_t value, const CMD cmd){
    send(static_cast<uint8_t>(CMD::CLR));
    sendNum(value);
    send(static_cast<uint8_t>(cmd));
}

