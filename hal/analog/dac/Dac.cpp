#include "Dac.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd::hal;
using namespace ymd::hal::DacUtils;

#ifdef ENABLE_DAC

void DacChannel::init(){
    DAC_InitTypeDef  DAC_InitType = {
        .DAC_Trigger = DAC_Trigger_None,
        .DAC_WaveGeneration = DAC_WaveGeneration_None,
        .DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0,
        .DAC_OutputBuffer = DAC_OutputBuffer_Disable,
    };

    DAC_Init(DAC_Channel_1, &DAC_InitType);
    DAC_Cmd(DAC_Channel_1, ENABLE);



    settle();
}

DacChannel &ymd::hal::DacChannel::operator=(const real_t value){
    const auto d = uni_to_u16(value);
    const auto align_mask = uint32_t(align_);

    switch(idx_){
        case ChannelNth::_1:
            DAC_SetChannel1Data(align_mask, d);
            break;
        case ChannelNth::_2:
            DAC_SetChannel2Data(align_mask, d);
            break;
    }

    return *this;
}



uint32_t DacChannel::get_channel_mask(const ChannelNth nth){
    switch(idx){
        default:
        case ChannelNth::_1:
            return DAC_Channel_1;
        case ChannelNth::_2:  
            return DAC_Channel_2;
    }
}


void DacChannel::settle(){
    auto & io = [this]() -> Gpio & {
        switch(idx_){
            default:
            case ChannelNth::_1:
                return portA[4];
            case ChannelNth::_2:
                return portA[5];
        }
    }();

    io.inana();
}

void Dac::enable_rcc(const Enable en){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, en);
}

void Dac::init(){
    enable_rcc(true);
}

#endif