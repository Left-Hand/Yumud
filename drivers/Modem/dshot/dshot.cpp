#include "dshot.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/dma/dma.hpp"
#include "core/utils/default.hpp"
#include <ranges>


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;



static constexpr auto DSHOT_LEN = DShotChannel::DSHOT_LEN;

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
    


    dma_channel_.start_transfer_mem2pph<uint16_t>(
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


static constexpr void dshot_fill_buf(const std::span<uint16_t, DSHOT_LEN> buf, const uint16_t bits){
    constexpr uint16_t HIGH_CVR = (234 * 2 / 3);
    constexpr uint16_t LOW_CVR = (234 * 1 / 3);
    uint16_t mut_bits = bits;
    for(size_t i = 0; i < 16; i++){
        buf[i] = (mut_bits & 0x8000) ? HIGH_CVR : LOW_CVR;
        mut_bits = mut_bits << 1;
    }
}


static constexpr uint16_t calculate_crc(uint16_t bits_in){
    const uint16_t speed_bits = bits_in << 5;
    const uint16_t bits = bits_in << 1;
    return speed_bits | static_cast<uint16_t>((bits ^ (bits >> 4) ^ (bits >> 8)) & 0x0f);
}

void DShotChannel::invoke(){
    burst_dma_pwm_.invoke();
}

void DShotChannel::init(){
    burst_dma_pwm_.set_buf(std::span(buf_));
    burst_dma_pwm_.install();
}
void DShotChannel::set_content(const uint16_t content_bits){
    if(content_bits > 2048) __builtin_trap();
    if(content_bits){
        auto crc = calculate_crc(content_bits);
        dshot_fill_buf(std::span(buf_), crc);
    }else{
        buf_.fill(0);
    }

    invoke();
}