#include "adc_lld.hpp"
#include "core/debug/debug.hpp"
#include "core/sdk.hpp"
#include "ral/adc.hpp"

using namespace ymd;

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, ADC_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::ADC_Def)>(x))

// #define ADC_ASSERT(cond, ...) if(bool(cond) == false) PANIC{__VA_ARGS__};
#define ADC_ASSERT(cond, ...)


namespace ymd::lld{

void adc_set_regular_trigger(void * p_inst, const hal::AdcRegularTrigger trigger){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.EXTSEL = static_cast<uint8_t>(trigger);
    tempreg.EXTTRIG = (trigger != hal::AdcRegularTrigger::SOFT);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void adc_set_injected_trigger(void * p_inst, const hal::AdcInjectedTrigger trigger){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
    tempreg.JEXTTRIG = (trigger != hal::AdcInjectedTrigger::SOFT);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}


void adc_set_regular_quantity(void * p_inst, const uint8_t quantity){
    ADC_ASSERT(quantity <= MAX_REGULAR_CHANNELS);
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.DISCNUM = quantity - 1;
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void adc_set_injected_quantity(void * p_inst, const uint8_t quantity){
    ADC_ASSERT(quantity <= MAX_INJECTED_CHANNELS);
    auto tempreg = std::bit_cast<ral::R32_ADC_ISQR>(SPL_INST(p_inst)->ISQR);
    tempreg.JL = quantity - 1;
    SPL_INST(p_inst)->ISQR = std::bit_cast<uint32_t>(tempreg);
}


void adc_enable_auto_inject(void * p_inst, const Enable en){
    ADC_AutoInjectedConvCmd(SPL_INST(p_inst), (en == EN));
}

void adc_configure_regular_channel(void * p_inst, const uint8_t rank, const hal::AdcChannelConfig & cfg){
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

void adc_configure_injected_channel(void * p_inst, const uint8_t rank, const hal::AdcChannelConfig & cfg){
    ADC_ASSERT(rank != 0, rank);
    ADC_ASSERT(rank <= MAX_INJECTED_CHANNELS, rank);

    ADC_InjectedChannelConfig(
        SPL_INST(p_inst),
        static_cast<uint8_t>(cfg.ch_sel),
        rank,
        static_cast<uint8_t>(cfg.sample_cycles)
    );
}


void adc_enable_dma(void * p_inst, const Enable en){
    RAL_INST(p_inst)->CTLR2.DMA = (en == EN);
}

void adc_enable_singleshot(void * p_inst, const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.DISCEN = (en == EN);
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void adc_enable_scan(void * p_inst, const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.SCAN = (en == EN);
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void adc_enable_temp_vref(void * p_inst, const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.TSVREFE = (en == EN);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}


void adc_set_mode(void * p_inst, const hal::AdcMode mode){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.DUALMOD = std::bit_cast<uint8_t>(mode);
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
};

void adc_set_pga(void * p_inst, const hal::AdcPga pga){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR1>(SPL_INST(p_inst)->CTLR1);
    tempreg.PGA = std::bit_cast<uint8_t>(pga);
    SPL_INST(p_inst)->CTLR1 = std::bit_cast<uint32_t>(tempreg);
}

void adc_enable_continous(void * p_inst, const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.CONT = (en == EN);
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}

void adc_enable_right_align(void * p_inst, const Enable en){
    auto tempreg = std::bit_cast<ral::R32_ADC_CTLR2>(SPL_INST(p_inst)->CTLR2);
    tempreg.ALIGN = en == DISEN;
    SPL_INST(p_inst)->CTLR2 = std::bit_cast<uint32_t>(tempreg);
}


void adc_cmd(void * p_inst, Enable en){
    ADC_Cmd(SPL_INST(p_inst), en == EN);
}

void adc_enable_rcc(const Nth adc_nth, Enable en){
    switch(adc_nth.count()){
        #ifdef RCC_APB2Periph_ADC1
        case 1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
            return;
        #endif

        #ifdef RCC_APB2Periph_ADC2
        case 2:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
            return;
        #endif
    }

    __builtin_trap();
}

void adc_enable_buffer(void * p_inst, Enable en){
    ADC_BufferCmd(SPL_INST(p_inst), en == EN);
}


}