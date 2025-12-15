#include "adc.hpp"
#include "core/debug/debug.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


void AdcPrimary::set_regular_channels(
    const std::initializer_list<AdcChannelConfig> & regular_list
){ 
    set_regular_count(regular_list.size());
    uint8_t i = 0;
    for(const auto & regular_cfg : regular_list){
        i++;
        ADC_RegularChannelConfig(
            SDK_INST(inst_),
            static_cast<uint8_t>(regular_cfg.sel),
            i,
            static_cast<uint8_t>(regular_cfg.cycles)
        );
        adc::details::install_pin(regular_cfg.sel);

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
                SDK_INST(inst_),
                static_cast<uint8_t>(injected_cfg.sel),
                i,
                static_cast<uint8_t>(injected_cfg.cycles));

            ADC_SetInjectedOffset(
                SDK_INST(inst_), ADC_InjectedChannel_1 + 
                (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data_, 0)); 
                // offset can`t be negative
            adc::details::install_pin(injected_cfg.sel);

            if(i > 4) break;
        }
    }
}
void AdcPrimary::init(
    const std::initializer_list<AdcChannelConfig> & regular_list,
    const std::initializer_list<AdcChannelConfig> & injected_list, 
    const Config & cfg
){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);	
    ADC_DeInit(SDK_INST(inst_));

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

    ADC_Init(SDK_INST(inst_), &ADC_InitStructure);

    bool temp_verf_activation = [&]{
        auto channel_is_temp_or_vref = [](const ChannelSelection sel){
            return sel == ChannelSelection::TEMP or
                    sel == ChannelSelection::VREF;
        };
        for(const auto injected_cfg : injected_list){
            if(channel_is_temp_or_vref(injected_cfg.sel))
                return true;
        }
        for(const auto regular_cfg : regular_list){
            if(channel_is_temp_or_vref(regular_cfg.sel))
                return true;
        }
        return false;
    }();


    set_regular_channels(regular_list);
    set_injected_channels(injected_list);

    if(temp_verf_activation) enable_temp_vref(EN);

    if(MAX(injected_list.size(), regular_list.size()) > 1){
        enable_scan(EN);  
    }else{
        enable_singleshot(EN);
    }


    ADC_ExternalTrigConvCmd(SDK_INST(inst_), ENABLE);
    ADC_ExternalTrigInjectedConvCmd(SDK_INST(inst_), ENABLE);

    ADC_DMACmd(SDK_INST(inst_), DISABLE);
    
    ADC_ClearITPendingBit(SDK_INST(inst_), ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(SDK_INST(inst_), ENABLE);

    ADC_Cmd(SDK_INST(inst_), ENABLE);

    {
        ADC_BufferCmd(SDK_INST(inst_), DISABLE);
        ADC_ResetCalibration(SDK_INST(inst_));
        while(ADC_GetResetCalibrationStatus(SDK_INST(inst_)));
        ADC_StartCalibration(SDK_INST(inst_));
        while(ADC_GetCalibrationStatus(SDK_INST(inst_)));
        cali_data_ = Get_CalibrationValue(SDK_INST(inst_));
    }

    ADC_BufferCmd(SDK_INST(inst_), ENABLE);
}


void AdcPrimary::set_mode(const Mode mode){
    auto tempreg = std::bit_cast<CTLR1>(SDK_INST(inst_)->CTLR1);
    tempreg.DUALMOD = std::bit_cast<uint8_t>(mode);
    SDK_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
};

void AdcPrimary::set_pga(const Pga pga){
    auto tempreg = std::bit_cast<CTLR1>(SDK_INST(inst_)->CTLR1);
    tempreg.PGA = std::bit_cast<uint8_t>(pga);
    SDK_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_continous(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SDK_INST(inst_)->CTLR2);
    tempreg.CONT = en == EN;
    SDK_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_auto_inject(const Enable en){
    ADC_AutoInjectedConvCmd(SDK_INST(inst_), en == EN);
}


void AdcPrimary::enable_right_align(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SDK_INST(inst_)->CTLR2);
    tempreg.ALIGN = en == DISEN;
    SDK_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    right_align_ = en == EN;
}

void AdcPrimary::set_regular_trigger(const RegularTrigger trigger){
    auto tempreg = std::bit_cast<CTLR2>(SDK_INST(inst_)->CTLR2);
    tempreg.EXTSEL = static_cast<uint8_t>(trigger);
    tempreg.EXTTRIG = (trigger != RegularTrigger::SW);
    SDK_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::set_injected_trigger(const InjectedTrigger trigger){
    auto tempreg = std::bit_cast<CTLR2>(SDK_INST(inst_)->CTLR2);
    tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
    tempreg.JEXTTRIG = (trigger != InjectedTrigger::SW);
    SDK_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::set_wdt_threshold(const uint16_t low,const uint16_t high){
    SDK_INST(inst_)->WDHTR = CLAMP(low, 0, get_max_value());
    SDK_INST(inst_)->WDLTR = CLAMP(high, 0, get_max_value());
}



void AdcPrimary::sw_start_regular(const bool force){
    if(force) set_regular_trigger(RegularTrigger::SW);
    ADC_SoftwareStartConvCmd(SDK_INST(inst_), true);
}

void AdcPrimary::sw_start_injected(const bool force){
    if(force) set_injected_trigger(InjectedTrigger::SW);
    ADC_SoftwareStartInjectedConvCmd(SDK_INST(inst_), true);
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
    ADC_DMACmd(SDK_INST(inst_), en == EN);
}

uint16_t AdcPrimary::get_conv_result(){
    return SDK_INST(inst_)->RDATAR;
}

uint32_t AdcPrimary::get_max_value() const {
    return ((1 << 12) - 1) << (right_align_ ? 0 : 4);
}

void AdcPrimary::set_regular_count(const uint8_t cnt){
    auto tempreg = std::bit_cast<CTLR1>(SDK_INST(inst_)->CTLR1);
    tempreg.DISCNUM = cnt;
    SDK_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    regular_cnt_ = cnt;
}

void AdcPrimary::set_injected_count(const uint8_t cnt){
    ADC_InjectedSequencerLengthConfig(SDK_INST(inst_), cnt);
    injected_cnt_ = cnt;
}

void AdcPrimary::set_regular_sample_cycles(const ChannelSelection sel,  const SampleCycles sample_cycles){
    uint8_t ch = std::bit_cast<uint8_t>(sel);
    uint8_t offset = ch % 10;
    offset *= 3;

    if(ch < 10){
        uint32_t tempreg = SDK_INST(inst_)->SAMPTR1;
        tempreg &= ~(0xb111 << offset);
        tempreg |= std::bit_cast<uint8_t>(sample_cycles) << offset;
        SDK_INST(inst_)->SAMPTR1 = tempreg;
    }else{
        uint32_t tempreg = SDK_INST(inst_)->SAMPTR2;
        tempreg &= ~(0xb111 << offset);
        tempreg |= std::bit_cast<uint8_t>(sample_cycles) << offset;
        SDK_INST(inst_)->SAMPTR2 = tempreg;
    }
}

void AdcPrimary::enable_singleshot(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SDK_INST(inst_)->CTLR1);
    tempreg.DISCEN = en == EN;
    SDK_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_scan(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SDK_INST(inst_)->CTLR1);
    tempreg.SCAN = en == EN;
    SDK_INST(inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_temp_vref(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SDK_INST(inst_)->CTLR2);
    tempreg.TSVREFE = en == EN;
    SDK_INST(inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::dyn_enable_interrupt(const AdcIT I, const Enable en){
    ADC_ITConfig(SDK_INST(inst_), std::bit_cast<uint16_t>(I), en == EN);
}
