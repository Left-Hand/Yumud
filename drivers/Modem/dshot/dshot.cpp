#include "dshot.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/dma/dma.hpp"

#include <ranges>


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

static constexpr auto DSHOT_LEN = DShotChannel::DSHOT_LEN;

void BurstDmaPwm::borrow(std::span<const uint16_t> pbuf){
    dma_channel_.init({DmaMode::ToPeriph, DmaPriority::Medium});
    pbuf_ = pbuf;
}


void BurstDmaPwm::invoke(){
    dma_channel_.start_transfer_mem2pph<uint16_t>(
        &timer_oc_.cvr(), pbuf_.data(), pbuf_.size()
    );
}

void BurstDmaPwm::install(){
    dma_channel_.init({DmaMode::ToPeriph, DmaPriority::Ultra});
    timer_oc_.init({});
    timer_oc_.enable_cvr_sync(EN);
}

bool BurstDmaPwm::is_done(){
    return dma_channel_.remaining() == 0;
}

uint32_t BurstDmaPwm::calc_cvr_from_duty(const uq32 dutycycle) const {
    return uint32_t((uint64_t(timer_oc_.arr()) * dutycycle.to_bits()) >> 32);
}

uq8 BurstDmaPwm::get_period_us() const{
    // return iq8(timer_oc_.arr() / 1000);
    
    return 0;
}

DShotChannel::DShotChannel(hal::TimerOC & oc):
    burst_dma_pwm_(oc){;}

BurstDmaPwm::BurstDmaPwm(hal::TimerOC & timer_oc):
    timer_oc_(timer_oc), 
    dma_channel_(timer_oc.dma().unwrap()){;}


void DShotChannel::update(const std::span<uint16_t, DSHOT_LEN> buf, const uint16_t data){
    uint16_t tempbuf = data;
    for(size_t i = 0; i < 16; i++){
        buf[i] = (tempbuf & 0x8000) ? HIGH_CVR : LOW_CVR;
        tempbuf = tempbuf << 1;
    }
}

WS2812_Phy_of_BurstPwm::WS2812_Phy_of_BurstPwm(BurstDmaPwm & burst_dma_pwm)
    : burst_dma_pwm_(burst_dma_pwm){}


void WS2812_Phy_of_BurstPwm::apply_mono_to_buf(
    const std::span<uint16_t, 8> buf, 
    uint8_t mono
) const{
    uint16_t HIGH_CVR = burst_dma_pwm_.calc_cvr_from_duty(uq32(0.85 / 1.25));
    uint16_t LOW_CVR = burst_dma_pwm_.calc_cvr_from_duty(uq32(0.4 / 1.25));

    for(size_t i = 0; i < 8; i++){
        buf[i] = (mono & 0x80) ? HIGH_CVR : LOW_CVR;
        mono = mono << 1;
    }
}

void WS2812_Phy_of_BurstPwm::apply_color_to_buf(
    std::span<uint16_t, 24> buf, 
    std::array<uint8_t, 3> color
) const{
    apply_mono_to_buf(std::span<uint16_t, 8>(buf.begin() + 0, 8), color[0]);
    apply_mono_to_buf(std::span<uint16_t, 8>(buf.begin() + 8, 8), color[1]);
    apply_mono_to_buf(std::span<uint16_t, 8>(buf.begin() + 16, 8), color[2]);
}

void DShotChannel::clear(const std::span<uint16_t, DSHOT_LEN> buf){
    for(auto & item : buf) item = 0;
}

uint16_t DShotChannel::calculate_crc(uint16_t data_in){
    uint16_t speed_data;
    speed_data = data_in << 5;
    data_in = data_in << 1;
    data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
    return speed_data | data_in;
}

void DShotChannel::invoke(){
    burst_dma_pwm_.invoke();
}

void DShotChannel::init(){
    burst_dma_pwm_.install();
}
void DShotChannel::set_dutycycle(const real_t dutycycle){
    if(dutycycle){
        auto crc = calculate_crc(MAX(int(dutycycle * 2047), 48));
        update(std::span(buf_), crc);
    }else{
        clear(std::span(buf_));
    }

    invoke();
}