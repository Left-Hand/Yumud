#include "dshot.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/dma/dma.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

void DShotChannel::update(uint16_t data){

    for(size_t i = 0; i < 16; i++){
        buf[i] = (data & 0x8000) ? high_cnt : low_cnt;
        data = data << 1;
    }
}

DShotChannel::DShotChannel(TimerOC & _oc):
        oc(_oc),
        dma_channel(_oc.dma())
        {;}

void DShotChannel::invoke(){
    dma_channel.transfer_mem2pph<uint16_t>((&oc.cvr()), buf, 40);
}

void DShotChannel::init(){
    dma_channel.init(DmaMode::toPeriph, DmaPriority::Ultra);
    oc.init().sync().enableDma();

    high_cnt = (234 * 2 / 3);
    low_cnt = (234 * 1 / 3);
}

DShotChannel & DShotChannel::operator = (const real_t duty){
    // DEBUG_PRINTLN(duty);
    if(duty) update(m_crc(MAX(int(duty * 2047), 48)));
    else update(0);
    // DEBUG_PRINTLN(buf);
    invoke();
    return *this;
}