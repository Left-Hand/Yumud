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

// #define ADC_ASSERT(cond, ...) if(bool(cond) == false) PANIC{__VA_ARGS__};
#define ADC_ASSERT(cond, ...)

static constexpr size_t MAX_REGULAR_CHANNELS = 16;
static constexpr size_t MAX_INJECTED_CHANNELS = 4;

using CTLR1 = ral::R32_ADC_CTLR1;
using CTLR2 = ral::R32_ADC_CTLR2;

void adc_set_regular_trigger(void * p_inst, const AdcRegularTrigger trigger){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.EXTSEL = static_cast<uint8_t>(trigger);
    tempreg.EXTTRIG = (trigger != AdcRegularTrigger::SOFT);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void adc_set_injected_trigger(void * p_inst, const AdcInjectedTrigger trigger){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
    tempreg.JEXTTRIG = (trigger != AdcInjectedTrigger::SOFT);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}


void adc_set_regular_count(void * p_inst, const uint8_t count){
    ADC_ASSERT(count <= MAX_REGULAR_CHANNELS);
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.DISCNUM = count - 1;
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void adc_set_injected_count(void * p_inst, const uint8_t count){
    ADC_ASSERT(count <= MAX_INJECTED_CHANNELS);
    auto tempreg = std::bit_cast<ral::R32_ADC_ISQR>(SPL_INST(p_inst)->ISQR);
    tempreg.JL = count - 1;
    SPL_INST(p_inst)->ISQR = std::bit_cast<uint32_t>(tempreg);
}



void adc_configure_regular_channel(void * p_inst, const size_t rank, const AdcChannelConfig & cfg){
    ADC_ASSERT(rank != 0);
    ADC_ASSERT(rank <= MAX_REGULAR_CHANNELS);

    #if 1
    ADC_RegularChannelConfig(
        SPL_INST(p_inst),
        static_cast<uint8_t>(cfg.ch_sel),
        rank,
        static_cast<uint8_t>(cfg.sample_cycles)
    );
    #else

    uint8_t ch = std::bit_cast<uint8_t>(cfg.ch_sel);
    uint8_t offset = ((ch > 10) ? ch - 10 : ch) * 3;
    volatile uint32_t & reg = ((ch > 10) ? SPL_INST(p_inst)->SAMPTR2 : SPL_INST(p_inst)->SAMPTR1);

    auto convert_mask = [&](uint32_t tempreg) -> uint32_t {
        tempreg &= ~(uint32_t(0b111 << offset));
        tempreg |= (std::bit_cast<uint8_t>(cfg.sample_cycles) << offset);
        return tempreg;
    };

    reg = convert_mask(static_cast<uint32_t>(reg));
    #endif
}

void adc_configure_injected_channel(void * p_inst, const size_t rank, const AdcChannelConfig & cfg){
    ADC_ASSERT(rank != 0, rank);
    ADC_ASSERT(rank <= MAX_INJECTED_CHANNELS, rank);

    ADC_InjectedChannelConfig(
        SPL_INST(p_inst),
        static_cast<uint8_t>(cfg.ch_sel),
        rank,
        static_cast<uint8_t>(cfg.sample_cycles)
    );
}

void AdcPrimary::set_regular_channels(
    const std::initializer_list<AdcChannelConfig> & regular_list
){ 
    adc_set_regular_count(p_inst_, regular_list.size());
    size_t idx = 0;
    for(const auto & regular_cfg : regular_list){
        adc_configure_regular_channel(p_inst_, idx + 1, regular_cfg);
        adc::details::install_pin(regular_cfg.ch_sel);

        idx++;
    }
}


void AdcPrimary::set_injected_channels(
    const std::initializer_list<AdcChannelConfig> & injected_list
){
    adc_set_injected_count(p_inst_, injected_list.size());
    uint8_t idx = 0;
    for(const auto & injected_cfg : injected_list){


        adc_configure_injected_channel(p_inst_, idx + 1, injected_cfg);

        ADC_SetInjectedOffset(
            SPL_INST(p_inst_), 
            ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (idx),
            
            // offset can`t be negative
            static_cast<uint16_t>(cali_data_)
        ); 

        adc::details::install_pin(injected_cfg.ch_sel);

        idx++;
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
    ADC_DeInit(SPL_INST(p_inst_));

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

    enable_temp_vref(EN);

    if(std::max(injected_list.size(), regular_list.size()) > 1){
        enable_scan(EN);  
    }else{
        enable_singleshot(EN);
    }


    ADC_ExternalTrigConvCmd(SPL_INST(p_inst_), ENABLE);
    ADC_ExternalTrigInjectedConvCmd(SPL_INST(p_inst_), ENABLE);

    ADC_DMACmd(SPL_INST(p_inst_), DISABLE);
    
    ADC_ClearITPendingBit(SPL_INST(p_inst_), ADC_IT_JEOC | ADC_IT_AWD | ADC_IT_EOC);
    
    ADC_AutoInjectedConvCmd(SPL_INST(p_inst_), ENABLE);

    ADC_Cmd(SPL_INST(p_inst_), ENABLE);

    {
        ADC_BufferCmd(SPL_INST(p_inst_), DISABLE);
        // ADC_ResetCalibration(SPL_INST(p_inst_));
        // while(ADC_GetResetCalibrationStatus(SPL_INST(p_inst_)));
        // ADC_StartCalibration(SPL_INST(p_inst_));
        // while(ADC_GetCalibrationStatus(SPL_INST(p_inst_)));
        cali_data_ = Get_CalibrationValue(SPL_INST(p_inst_));
        ADC_BufferCmd(SPL_INST(p_inst_), ENABLE);
    }
}


void AdcPrimary::set_mode(const Mode mode){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst_)->CTLR1);
    tempreg.DUALMOD = std::bit_cast<uint8_t>(mode);
    SPL_INST(p_inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
};

void AdcPrimary::set_pga(const Pga pga){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst_)->CTLR1);
    tempreg.PGA = std::bit_cast<uint8_t>(pga);
    SPL_INST(p_inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_continous(const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst_)->CTLR2);
    tempreg.CONT = (en == EN);
    SPL_INST(p_inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
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
    ADC_AutoInjectedConvCmd(SPL_INST(p_inst_), (en == EN));
}


void AdcPrimary::enable_right_align(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(p_inst_)->CTLR2);
    tempreg.ALIGN = en == DISEN;
    SPL_INST(p_inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    left_aligned_ = (en == DISEN);
}




void AdcPrimary::set_regular_trigger(const RegularTrigger trigger){
    adc_set_regular_trigger(p_inst_, trigger);
}

void AdcPrimary::set_injected_trigger(const InjectedTrigger trigger){
    adc_set_injected_trigger(p_inst_, trigger);
}

void AdcPrimary::set_wdt_threshold(const uint16_t low,const uint16_t high){
    SPL_INST(p_inst_)->WDLTR = std::clamp<uint32_t>(low, 0u, get_max_value());
    SPL_INST(p_inst_)->WDHTR = std::clamp<uint32_t>(high, 0u, get_max_value());
}



void AdcPrimary::sw_start_regular(const bool force){
    if(force) set_regular_trigger(RegularTrigger::SOFT);
    ADC_SoftwareStartConvCmd(SPL_INST(p_inst_), true);
}

void AdcPrimary::sw_start_injected(const bool force){
    if(force) set_injected_trigger(InjectedTrigger::SOFT);
    ADC_SoftwareStartInjectedConvCmd(SPL_INST(p_inst_), true);
}

void AdcPrimary::enable_dma(const Enable en){
    RAL_INST(p_inst_)->CTLR2.DMA = (en == EN);
}

void AdcPrimary::enable_singleshot(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(p_inst_)->CTLR1);
    tempreg.DISCEN = (en == EN);
    SPL_INST(p_inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_scan(const Enable en){
    auto tempreg = std::bit_cast<CTLR1>(SPL_INST(p_inst_)->CTLR1);
    tempreg.SCAN = (en == EN);
    SPL_INST(p_inst_)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::enable_temp_vref(const Enable en){
    auto tempreg = std::bit_cast<CTLR2>(SPL_INST(p_inst_)->CTLR2);
    tempreg.TSVREFE = (en == EN);
    SPL_INST(p_inst_)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void AdcPrimary::dyn_enable_interrupt(const AdcIT I, const Enable en){
    ADC_ITConfig(SPL_INST(p_inst_), std::bit_cast<uint16_t>(I), (en == EN));
}
