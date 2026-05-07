#include "dshot.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/dma/dma.hpp"
#include "core/utils/default.hpp"
#include <ranges>


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
using namespace ymd::drivers::dshot;

void BurstDmaPwm::set_buf(std::span<const uint16_t> pbuf){
    pbuf_ = pbuf;
}


void BurstDmaPwm::invoke(){
    // PANIC{"hyw"};
    // PANIC{"hyw", std::hex, (uint32_t)&timer_oc_.cvr()};
    dma_channel_.init({
        .mode = DmaMode::BurstMemoryToPeriph, 
        .priority = DmaPriority::Ultra
    });
    


    dma_channel_.start_transfer_mem2pph<
        DmaWordSize::TwoByte, DmaWordSize::TwoByte
    >(
        &timer_oc_.cvr(), pbuf_.data(), pbuf_.size()
    );
}

void BurstDmaPwm::install(){
    // PANIC{"hyw"};
    dma_channel_.init({
        .mode = DmaMode::BurstMemoryToPeriph, 
        .priority = DmaPriority::Ultra
    });
    
    timer_oc_.init(Default);
    timer_oc_.enable_cvr_sync(EN);
}

bool BurstDmaPwm::is_done(){
    return dma_channel_.pending_count() == 0;
}



DShotChannel::DShotChannel(hal::TimerOC & oc):
    burst_dma_pwm_(oc){;}

BurstDmaPwm::BurstDmaPwm(hal::TimerOC & timer_oc):
    timer_oc_(timer_oc), 
    dma_channel_(timer_oc.dma().unwrap()){;}





void DShotChannel::invoke(){
    burst_dma_pwm_.invoke();
}

void DShotChannel::init(){
    burst_dma_pwm_.set_buf(std::span(buf_));
    burst_dma_pwm_.install();
}
void DShotChannel::set_content(const uint16_t content_bits){
    DshotContentBuilder::from_default().exact_to(std::span(buf_), content_bits);

    invoke();
}