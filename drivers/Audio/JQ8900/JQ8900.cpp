#include "JQ8900.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

void JQ8900::Phy::write(const uint8_t data){
    ser_.set_high();
    clock::delay(1000us);
    ser_.set_low();
    clock::delay(3200us);

    for(uint8_t i = 1; bool(i); i <<= 1){
        ser_.set_high();
        if(i & data){
            clock::delay(600us);
            ser_.set_low();
            clock::delay(200us);
        }else{
            clock::delay(200us);
            ser_.set_low();
            clock::delay(600us);  
        }
    }
    ser_.set_high();
}

void JQ8900::send_num(const int value){
    if(value > 10){
        transport_.write(value / 10);
        transport_.write(value % 10);
    }else{
        transport_.write(value);
    }
}

void JQ8900::send_line(const uint8_t value, const Command cmd){
    transport_.write(static_cast<uint8_t>(Command::Clear));
    send_num(value);
    transport_.write(static_cast<uint8_t>(cmd));
}

