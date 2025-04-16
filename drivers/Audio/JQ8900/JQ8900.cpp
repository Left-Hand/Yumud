#include "JQ8900.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

void JQ8900::Phy::write(const uint8_t data){
    ser_ = true;
    udelay(1000);
    ser_ = false;
    udelay(3200);

    for(uint8_t i = 1; bool(i); i <<= 1){
        ser_ = true;
        if(i & data){
            udelay(600);
            ser_ = false;
            udelay(200);
        }else{
            udelay(200);
            ser_ = false;
            udelay(600);  
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

