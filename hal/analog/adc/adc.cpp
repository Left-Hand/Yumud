#include "adc.hpp"
#include "adc_lld.hpp"

#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;



#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::ADC_Def)>(x))



namespace{

template<typename T>
[[nodiscard]] static constexpr T _clamp(T x, T mi, T ma){
    if(x > ma) return ma;
    if(x < mi) return mi;
    return x;
}

}

void AdcPrimary::set_regular_channels(
    const std::initializer_list<AdcChannelConfig> & regular_list
){ 
    lld::adc_set_regular_quantity(p_inst_, regular_list.size());
    for(size_t i = 0; i < regular_list.size(); i++){
        auto & regular_cfg = regular_list.begin()[i];
        lld::adc_configure_regular_channel(p_inst_, i + 1, regular_cfg);
        adc::details::install_pin(regular_cfg.ch_sel);
    }
}


void AdcPrimary::set_injected_channels(
    const std::initializer_list<AdcChannelConfig> & injected_list
){
    lld::adc_set_injected_quantity(p_inst_, injected_list.size());
    // uint8_t idx = 0;
    for(size_t i = 0; i < injected_list.size(); i++){
        const auto & injected_cfg  = injected_list.begin()[i];

        lld::adc_configure_injected_channel(p_inst_, i + 1, injected_cfg);

        ADC_SetInjectedOffset(
            SPL_INST(p_inst_), 
            ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i),
            
            // offset can`t be negative
            static_cast<uint16_t>(cali_bvalue_)
        ); 
        adc::details::install_pin(injected_cfg.ch_sel);
    }
}


void AdcPrimary::init(
    const std::initializer_list<AdcChannelConfig> & regular_list,
    const std::initializer_list<AdcChannelConfig> & injected_list, 
    const Config & cfg
){
    (void)cfg;
    
    ADC_DeInit(SPL_INST(p_inst_));

    lld::adc_enable_rcc(1_nth, EN);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);	

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

    ADC_Init(SPL_INST(p_inst_), &ADC_InitStructure);

    #if 0
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
    #endif


    set_regular_channels(regular_list);
    set_injected_channels(injected_list);

    lld::adc_enable_temp_vref(p_inst_, EN);

    if(std::max(injected_list.size(), regular_list.size()) > 1){
        lld::adc_enable_scan(p_inst_, EN);  
    }else{
        lld::adc_enable_scan(p_inst_, DISEN);  
    }

    ADC_ExternalTrigConvCmd(SPL_INST(p_inst_), ENABLE);
    ADC_ExternalTrigInjectedConvCmd(SPL_INST(p_inst_), ENABLE);

    lld::adc_enable_dma(p_inst_, DISEN);
    
    ADC_ClearITPendingBit(SPL_INST(p_inst_), ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(SPL_INST(p_inst_), ENABLE);

    lld::adc_enable_auto_inject(p_inst_, EN);


    // {
    //     lld::adc_enable_buffer(p_inst_, DISEN);
    //     // ADC_ResetCalibration(SPL_INST(p_inst_));
    //     // while(ADC_GetResetCalibrationStatus(SPL_INST(p_inst_)));
    //     // ADC_StartCalibration(SPL_INST(p_inst_));
    //     // while(ADC_GetCalibrationStatus(SPL_INST(p_inst_)));
    //     cali_bvalue_ = Get_CalibrationValue(SPL_INST(p_inst_));
    //     ADC_BufferCmd(SPL_INST(p_inst_), ENABLE);
    //     lld::adc_enable_buffer(p_inst_, EN);
    // }
}


void AdcPrimary::set_mode(const Mode mode){
    lld::adc_set_mode(p_inst_, mode);
};

void AdcPrimary::set_pga(const Pga pga){
    lld::adc_set_pga(p_inst_, pga);
}

void AdcPrimary::enable_continous(const Enable en){
    lld::adc_enable_continous(p_inst_, en);
}

void AdcPrimary::enable_right_align(const Enable en){
    lld::adc_enable_right_align(p_inst_, en);
    left_aligned_ = (en == DISEN);
}



uint16_t AdcPrimary::regular_conv_result(){
    return SPL_INST(p_inst_)->RDATAR;
}

uint16_t AdcPrimary::injected_conv_result(const size_t rank){
    switch(rank){
        case 1: return SPL_INST(p_inst_)->IDATAR1;
        case 2: return SPL_INST(p_inst_)->IDATAR2;
        case 3: return SPL_INST(p_inst_)->IDATAR3;
        case 4: return SPL_INST(p_inst_)->IDATAR4;
    }
    __builtin_abort();
}

void AdcPrimary::enable_auto_inject(const Enable en){
    lld::adc_enable_auto_inject(p_inst_, en);
}






void AdcPrimary::set_regular_trigger(const RegularTrigger trigger){
    lld::adc_set_regular_trigger(p_inst_, trigger);
}

void AdcPrimary::set_injected_trigger(const InjectedTrigger trigger){
    lld::adc_set_injected_trigger(p_inst_, trigger);
}



void AdcPrimary::set_wdt_threshold(const uint16_t low, const uint16_t high){
    SPL_INST(p_inst_)->WDLTR = _clamp<uint32_t>(low, 0u, get_max_bvalue());
    SPL_INST(p_inst_)->WDHTR = _clamp<uint32_t>(high, 0u, get_max_bvalue());
}

void AdcPrimary::register_nvic(const NvicPriorityCode priority, const Enable en){
    lld::nvic_set_irqn_priority(ADC_IRQn, priority);
    lld::nvic_enable_irqn(ADC_IRQn, en == EN);
}


void AdcPrimary::sw_start_regular(){
    ADC_SoftwareStartConvCmd(SPL_INST(p_inst_), true);
}

void AdcPrimary::sw_start_injected(){
    ADC_SoftwareStartInjectedConvCmd(SPL_INST(p_inst_), true);
}

void AdcPrimary::enable_dma(const Enable en){
    lld::adc_enable_dma(p_inst_, en);
}

void AdcPrimary::enable_singleshot(const Enable en){
    lld::adc_enable_singleshot(p_inst_, en);
}

void AdcPrimary::enable_scan(const Enable en){
    lld::adc_enable_scan(p_inst_, en);
}

void AdcPrimary::enable_temp_vref(const Enable en){
    lld::adc_enable_temp_vref(p_inst_, en);
}


void AdcPrimary::enable_interrupt(const AdcIT I, const Enable en){
    ADC_ITConfig(SPL_INST(p_inst_), std::bit_cast<uint16_t>(I), (en == EN));
}
