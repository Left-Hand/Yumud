#include "JQ8900.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

void JQ8900::Phy::write(const uint8_t data){
    ser_ = true;
    delayMicroseconds(1000);
    ser_ = false;
    delayMicroseconds(3200);
    // DEBUG_PRINTLN(data);

    // for(size_t i = 0; i < 8; i++){
    //     if((1 << i) & data){
    //         ser_ = true;
    //         delayMicroseconds(600);
    //         ser_ = false;
    //         delayMicroseconds(200);
    //     }else{
    //         ser_ = true;
    //         delayMicroseconds(200);
    //         ser_ = false;
    //         delayMicroseconds(600);  
    //     }
    // }
    for(uint8_t i = 1; bool(i); i <<= 1){
        ser_ = true;
        if(i & data){
            delayMicroseconds(600);
            ser_ = false;
            delayMicroseconds(200);
        }else{
            delayMicroseconds(200);
            ser_ = false;
            delayMicroseconds(600);  
        }
    }
    ser_ = true;
}

void JQ8900::send_num(const int value){
    if(value > 10){
        phy_.write(value / 10);
        phy_.write(value % 10);
    }else{
        phy_.write(value);
    }
}

void JQ8900::send_line(const uint8_t value, const Command cmd){
    phy_.write(static_cast<uint8_t>(Command::Clear));
    send_num(value);
    phy_.write(static_cast<uint8_t>(cmd));
}

