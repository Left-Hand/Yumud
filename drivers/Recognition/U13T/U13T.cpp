#include "U13T.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd::drivers;



uint8_t Matcher::matchID(const uint8_t * iptr){
    for(uint8_t i = 0; i < 16; i++){
        if(!marked[i]){
            bool ok = true;
            for(uint8_t j = 0; j < 16; j++){
                if(item[i][j] != *(iptr + j))
                    ok = false;
            }
            if(ok){
                marked[i] = true;
                DEBUG_PRINTLN("%d marked\r\n", i);
                return i+1;
            }
        }
    }
    return 0;
}


void U13T::init(){
    // initGpio();
    // initUart();

    // serPin.outpp();
    // ledPin.outpp();
    // player.init();

    clearBuffer();
}

bool U13T::checkNew(){
    bool hasNew = (dead_ticks == 0);
    if(hasNew){
        DEBUG_PRINTLN("new:");
        for(uint8_t & ch:buffer)
            DEBUG_PRINTLN("%d ", ch);
        DEBUG_PRINTLN("\r\n");

        recv = buffer;
        clearBuffer();
    }
    return hasNew;
}

void U13T::lineCb(){
    if(recv.size() == 12){
        constexpr auto cmds = std::to_array<char>({0x7f, 0x04, 0x00, 0x11, 0x04, 0x11});
        uart_.writeN((const char *)cmds.begin(), (size_t)cmds.size());
    }else{
        uint8_t id = matcher.matchID(&recv[11]);

        if(id != 4 && id != 5 && id != 10 && id != 12 && id !=13 && id !=15){
            if(id == 6 || id == 14){
                // player.setDisc(id);
            }
            ledAlive = 300;
        }
    }

}

void U13T::tick(){
    if(dead_ticks > -1) dead_ticks--;
    if(checkNew()) lineCb();
    // ledPin = bool(ledAlive > 0);
    ledAlive = std::max(ledAlive - 1, 0);
}

void U13T::clearBuffer(){
    buffer.reserve(64);
    buffer.clear();
}

void U13T::update(){
    if(uart_.available()){
        char chr;
        uart_.read1(chr);
        buffer.push_back(chr);
        dead_ticks = dead_limit;
    }

}