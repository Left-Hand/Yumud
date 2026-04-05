#include "adc.hpp"
#include "core/debug/debug.hpp"
#include "core/sdk.hpp"
#include "ral/adc.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::ADC_Def)>(x))


void AdcPrimary::set_regular_channels(
    const std::initializer_list<AdcChannelConfig> & regular_list
){ 
    set_regular_count(regular_list.size());
    uint8_t i = 0;
    for(const auto & regular_cfg : regular_list){
        i++;
        ADC_RegularChannelConfig(
            SPL_INST(inst_),
            static_cast<uint8_t>(regular_cfg.ch_sel),
            i,
            static_cast<uint8_t>(regular_cfg.cycles)
        );
        adc::details::install_pin(regular_cfg.ch_sel);

        if(i > 16) break;
    }
}


void AdcPrimary::set_injected_channels(
    const std::initializer_list<AdcChannelConfig> & injected_list
){
    {
        set_injected_count(injected_list.size());
        uint8_t i = 0;
        for(const auto & injected_cfg : injected_list){
            i++;

            ADC_InjectedChannelConfig(
                SPL_INST(inst_),
                static_cast<uint8_t>(injected_cfg.ch_sel),
                i,
                static_cast<uint8_t>(injected_cfg.cycles));

            ADC_SetInjectedOffset(
                SPL_INST(inst_), 
                ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),
                
                // offset can`t be negative
                static_cast<uint16_t>(std::max<int>(cali_data_, 0))
            ); 
            adc::details::install_pin(injected_cfg.ch_sel);

            if(i > 4) break;
        }
    }
}
void AdcPrimary::init(
    const std::initializer_list<AdcChannelConfig> & regular_list,
    const std::initializer_list<AdcChannelConfig> & injected_list, 
    const Config & cfg
){
    (void)cfg;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);	
    ADC_DeInit(SPL_INST(inst_));

    const ADC_InitTypeDef ADC_InitStructure = {
        .ADC_Mode = ADC_Mode_Independent,
        .ADC_ScanConvMode = DISABLE,
        .ADC_ContinuousConvMode = DISABLE,
        .ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,
        .ADC_DataAlign = ADC_DataAlign_Right,
        .ADC_NbrOfChannel = 1,
        .ADC_OutputBuffer = ADC_OutputBuffer_Disable,
        .ADC_Pga = ADC_Pga_1,
    };

    ADC_Init(SPL_INST(inst_), &ADC_InitStructure);

    bool temp_verf_activation = [&]{
        auto channel_is_temp_or_vref = [](const ChannelSelection ch_sel){
            return ch_sel == ChannelSelection::TEMP or
                    ch_sel == ChannelSelection::VREF;
        };
        for(const auto injected_cfg : injected_list){
            if(channel_is_temp_or_vref(injected_cfg.ch_sel))
                return true;
        }
        for(const auto regular_cfg : regular_list){
            if(channel_is_temp_or_vref(regular_cfg.ch_sel))
                return true;
        }
        return false;
    }();


    set_regular_channels(regular_list);
    set_injected_channels(injected_list);

    if(temp_verf_activation) enable_temp_vref(EN);

    if(std::max(injected_list.size(), regular_list.size()) > 1){
        enable_scan(EN);  
    }else{
        enable_singleshot(EN);
    }


    ADC_ExternalTrigConvCmd(SPL_INST(inst_), ENABLE);
    ADC_ExternalTrigInjectedConvCmd(SPL_INST(inst_), ENABLE);

    ADC_DMACmd(SPL_INST(inst_), DISABLE);
    
    ADC_ClearITPendingBit(SPL_INST(inst_), ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(SPL_INST(inst_), ENABLE);

    ADC_Cmd(SPL_INST(inst_), ENABLE);

    {
        ADC_BufferCmd(SPL_INST(inst_), DISABLE);
        ADC_ResetCalibration(SPL_INST(inst_));
        while(ADC_GetResetCalibrationStatus(SPL_INST(inst_)));
        ADC_StartCalibration(SPL_INST(inst_));
        while(ADC_GetCalibrationStatus(SPL_INST(inst_)));
        cali_data_ = Get_CalibrationValue(SPL_INST(inst_));
    }

    ADC_BufferCmd(SPL_INST(inst_), ENABLE);
}


void AdcPrimary::set_mode(const Mode mode){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(inst_)->CTLR1);
    tempreg.DUALMOD = std::bit_cast<uint8_t>(mode);
    SPL_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
};

void AdcPrimary::set_pga(const Pga pga){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(inst_)->CTLR1);
    tempreg.PGA = std::bit_cast<uint8_t>(pga);
    SPL_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_continous(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(inst_)->CTLR2);
    tempreg.CONT = (en == EN);
    SPL_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_auto_inject(const Enable en){
    ADC_AutoInjectedConvCmd(SPL_INST(inst_), (en == EN));
}


void AdcPrimary::enable_right_align(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(inst_)->CTLR2);
    tempreg.ALIGN = en == DISEN;
    SPL_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    left_aligned_ = (en == DISEN);
}

void AdcPrimary::set_regular_trigger(const RegularTrigger trigger){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(inst_)->CTLR2);
    tempreg.EXTSEL = static_cast<uint8_t>(trigger);
    tempreg.EXTTRIG = (trigger != RegularTrigger::SW);
    SPL_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::set_injected_trigger(const InjectedTrigger trigger){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(inst_)->CTLR2);
    tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
    tempreg.JEXTTRIG = (trigger != InjectedTrigger::SW);
    SPL_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::set_wdt_threshold(const uint16_t low,const uint16_t high){
    SPL_INST(inst_)->WDLTR = std::clamp<uint32_t>(low, 0u, get_max_value());
    SPL_INST(inst_)->WDHTR = std::clamp<uint32_t>(high, 0u, get_max_value());
}



void AdcPrimary::sw_start_regular(const bool force){
    if(force) set_regular_trigger(RegularTrigger::SW);
    ADC_SoftwareStartConvCmd(SPL_INST(inst_), true);
}

void AdcPrimary::sw_start_injected(const bool force){
    if(force) set_injected_trigger(InjectedTrigger::SW);
    ADC_SoftwareStartInjectedConvCmd(SPL_INST(inst_), true);
}

[[nodiscard]] bool AdcPrimary::is_regular_idle(){
    return ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC);
}

[[nodiscard]] bool AdcPrimary::is_injected_idle(){
    return ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC);
}

[[nodiscard]] bool AdcPrimary::is_idle(){
    return (is_regular_idle() && is_injected_idle());
}

void AdcPrimary::enable_dma(const Enable en){
    // ADC_DMACmd(SPL_INST(inst_), (en == EN));
    RAL_INST(inst_)->CTLR2.DMA = (en == EN);
}

uint16_t AdcPrimary::get_conv_result(){
    return SPL_INST(inst_)->RDATAR;
}


void AdcPrimary::set_regular_count(const uint8_t cnt){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(inst_)->CTLR1);
    tempreg.DISCNUM = cnt;
    SPL_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    num_regular_ = cnt;
}

void AdcPrimary::set_injected_count(const uint8_t cnt){
    ADC_InjectedSequencerLengthConfig(SPL_INST(inst_), cnt);
    num_injected_ = cnt;
}

void AdcPrimary::set_regular_sample_cycles(const ChannelSelection ch_sel,  const SampleCycles sample_cycles){
    uint8_t ch = std::bit_cast<uint8_t>(ch_sel);
    uint8_t offset = ((ch > 10) ? ch - 10 : ch) * 3;
    volatile uint32_t & reg = ((ch > 10) ? SPL_INST(inst_)->SAMPTR2 : SPL_INST(inst_)->SAMPTR1);

    auto convert_mask = [&](uint32_t tempreg) -> uint32_t {
        tempreg &= ~(uint32_t(0b111 << offset));
        tempreg |= (std::bit_cast<uint8_t>(sample_cycles) << offset);
        return tempreg;
    };

    reg = convert_mask(static_cast<uint32_t>(reg));
}

void AdcPrimary::enable_singleshot(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(inst_)->CTLR1);
    tempreg.DISCEN = (en == EN);
    SPL_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_scan(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(inst_)->CTLR1);
    tempreg.SCAN = (en == EN);
    SPL_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_temp_vref(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(inst_)->CTLR2);
    tempreg.TSVREFE = (en == EN);
    SPL_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::dyn_enable_interrupt(const AdcIT I, const Enable en){
    ADC_ITConfig(SPL_INST(inst_), std::bit_cast<uint16_t>(I), (en == EN));
}
