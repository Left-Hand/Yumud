#include "JQ8900.hpp"

using namespace ymd::drivers;

void JQ8900::send(const uint8_t data){
    ser = true;
    delayMicroseconds(1000);
    ser = false;
    delayMicroseconds(3200);

    for(size_t i = 1; bool(i); i <<= 1){
        ser = true;
        if(i & data){
            delayMicroseconds(600);
            ser = false;
            delayMicroseconds(200);
        }else{
            delayMicroseconds(200);
            ser = false;
            delayMicroseconds(600);  
        }
    }
    ser = true;
}

void JQ8900::sendLine(const uint8_t value, const CMD cmd){
    send(static_cast<uint8_t>(CMD::CLR));
    if(value > 10){
        send(value / 10);
        send(value % 10);
    }else{
        send(value);
    }
    send(static_cast<uint8_t>(cmd));
}

