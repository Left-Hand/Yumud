#include "dshot.hpp"

void DShotChannel::update(uint16_t data){
    uint16_t data_in = data;
    uint8_t i;
    for(i=0;i<16;i++){
        buf[i] = (data_in & 0x8000) ? high_cnt : low_cnt;
        data_in = data_in << 1;
    }
}

void DShotChannel::invoke(){
    dma_channel.begin((void *)&oc.cvr(), (void *)buf.begin(), buf.size());
}


void DShotChannel::init(){
    dma_channel.init(DmaChannel::Mode::toPeriph, DmaChannel::Priority::ultra);
    dma_channel.configDataBytes(2);
    oc.init();
    // oc.enableSync();
    oc.enableDma();
    buf.fill(0);
}