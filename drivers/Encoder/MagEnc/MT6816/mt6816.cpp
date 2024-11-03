#include "mt6816.hpp"


using namespace yumud::drivers;
using namespace yumud;



void MT6816::init() {
    last_semantic = 0;
    lap_position = real_t(-1); // not possible before init done;
    while(getLapPosition() < 0){this->update();} // while reading before get correct position
}

uint16_t MT6816::getPositionData(){
    uint16_t dataTx[2];
    uint16_t dataRx[2] = {0, 0};

    dataTx[0] = (0x80 | 0x03) << 8;
    dataTx[1] = (0x80 | 0x04) << 8;

    bus_drv.transfer(dataRx[0], dataTx[0]);
    bus_drv.transfer(dataRx[1], dataTx[1]);

    return((dataRx[0] & 0x00FF) << 8) | (dataRx[1]);
}

void MT6816::update() {
    uint16_t raw = getPositionData();
    if(fast_mode == false){
        last_semantic = raw;

        uint8_t count = 0;

        raw -= last_semantic.pc;
        while(raw){//Brian Kernighan algorithm
            raw &= raw - 1;
            ++count;
        }

        if(count % 2 == last_semantic.pc){
            u16_to_uni(last_semantic.data_14bit << 2, lap_position);
        }else{
            errcnt++;
        }
    }else{
        u16_to_uni(raw, lap_position);
    }
}
