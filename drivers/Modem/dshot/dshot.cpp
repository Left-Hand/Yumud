#include "dshot.hpp"

void DShotChannel::update(uint16_t data){

    for(size_t i = 0; i < 16; i++){
        buf[i] = (data & 0x8000) ? high_cnt : low_cnt;
        data = data << 1;
    }
}

void DShotChannel::invoke(){
    dma_channel.begin((void *)&oc.cvr(), (void *)buf.begin(), buf.size());
}

void DShotChannel::init(){
    dma_channel.init(DmaChannel::Mode::toPeriph, DmaChannel::Priority::ultra);
    dma_channel.configDataBytes(2);
    oc.init();
    oc.enableSync();
    oc.enableDma();

    high_cnt = (234 * 2 / 3);
    low_cnt = (234 * 1 / 3);
}