#pragma once

#include "core/platform.hpp"
#include <initializer_list>
#include "hal/nvic/nvic.hpp"
#include "regular_channel.hpp"
#include "injected_channel.hpp"
#include "adc_utils.hpp"

#if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
extern "C"{
__interrupt void ADC1_2_IRQHandler(void);
}
#endif


namespace ymd::hal{

class DmaChannel;

enum class AdcEvent:uint8_t{
    EndOfConversion,
    EndOfInjectedConversion,
    AnalogWatchdog
};

struct Adc_Prelude{
    using Callback = std::function<void(AdcEvent)>;

    struct CTLR1{
        uint32_t AWDCH:5;
        uint32_t EOCIE:1;
        uint32_t AWDIE:1;
        uint32_t JEOCIE:1;

        uint32_t SCAN:1;
        uint32_t AWDSGL:1;
        uint32_t JAUTO:1;
        uint32_t DISCEN:1;
        uint32_t JDISCEN:1;
        uint32_t DISCNUM:3;

        uint32_t DUALMOD:4;
        uint32_t __RESV1__:2;
        uint32_t JAWDEN:1;
        uint32_t AWDEN:1;

        uint32_t TKENABLE:1;
        uint32_t TKITUNE:1;
        uint32_t BUFEN:1;
        uint32_t PGA:2;
        uint32_t __RESV2__:3;
    };


    struct CTLR2{
        union{
            struct{
                uint32_t ADON:1;
                uint32_t CONT:1;
                uint32_t CAL:1;
                uint32_t RSTCAL:1;
                uint32_t __RESV1__:4;

                uint32_t DMA:1;
                uint32_t __RESV2__:2;
                uint32_t ALIGN:1;
                uint32_t JEXTSEL:3;
                uint32_t JEXTTRIG:1;

                uint32_t __RESV3__:1;
                uint32_t EXTSEL:3;
                uint32_t EXTTRIG:1;
                uint32_t JSWSTART:1;
                uint32_t SWSTART:1;
                uint32_t TSVREFE:1;

                uint32_t __RESV4__:8;
            };
            uint32_t data;
        };
    };


    using Pga = AdcPga;
    using RegularTrigger = AdcRegularTrigger;
    using InjectedTrigger = AdcInjectedTrigger;
    
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;
    using Mode = AdcMode;
    using IT = AdcIT;
};

struct AdcChannelConfig{
    using ChannelSelection = AdcChannelSelection;
    using SampleCycles = AdcSampleCycles;

    ChannelSelection nth;
    SampleCycles cycles;
};




class AdcPrimary: public Adc_Prelude{
public:
    explicit AdcPrimary(ADC_TypeDef * inst):
        inst_(inst),
        injected_channels{
            AdcInjectedChannel(inst_, AdcChannelSelection::VREF, 1),
            AdcInjectedChannel(inst_, AdcChannelSelection::VREF, 2),
            AdcInjectedChannel(inst_, AdcChannelSelection::VREF, 3),
            AdcInjectedChannel(inst_, AdcChannelSelection::VREF, 4)
        }{;}


    struct Config{
        Mode mode = Mode::Independent;
    };

    void init(
        const std::initializer_list<AdcChannelConfig> & regular_list,
        const std::initializer_list<AdcChannelConfig> & injected_list, 
        const Config & cfg);

    template<size_t I>
    requires ((I >= 1) and (I <= 4))
    AdcInjectedChannel & inj(){
        return injected_channels[I - 1];
    }


    template<typename Fn>
    void set_event_handler(Fn && cb){
        callback_ = std::forward<Fn>(cb);
    }

    void register_nvic(const NvicPriority priority, const Enable en){
        priority.with_irqn(ADC_IRQn).enable(en);
    }

    template<IT I>
    void enable_interrupt(const Enable en){
        ADC_ITConfig(inst_, std::bit_cast<uint16_t>(I), en == EN);
    }


    void set_mode(const Mode mode){
        auto tempreg = std::bit_cast<CTLR1>(inst_->CTLR1);
        tempreg.DUALMOD = std::bit_cast<uint8_t>(mode);
        inst_->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    };

    void set_pga(const Pga pga){
        auto tempreg = std::bit_cast<CTLR1>(inst_->CTLR1);
        tempreg.PGA = std::bit_cast<uint8_t>(pga);
        inst_->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_continous(const Enable en){
        auto tempreg = std::bit_cast<CTLR2>(inst_->CTLR2);
        tempreg.CONT = en == EN;
        inst_->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_auto_inject(const Enable en){
        ADC_AutoInjectedConvCmd(inst_, en == EN);
    }


    void set_regular_channels(const std::initializer_list<AdcChannelConfig> & regular_list);
    void set_injected_channels(const std::initializer_list<AdcChannelConfig> & injected_list);

    void enable_right_align(const Enable en){
        CTLR2 tempreg;
        tempreg.data = inst_->CTLR2;
        tempreg.ALIGN = en == DISEN;
        inst_->CTLR2 = tempreg.data;
        right_align = en == EN;
    }

    void set_regular_trigger(const RegularTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = inst_->CTLR2;
        tempreg.EXTSEL = static_cast<uint8_t>(trigger);
        tempreg.EXTTRIG = (trigger != RegularTrigger::SW);
        inst_->CTLR2 = tempreg.data;
    }

    void set_injected_trigger(const InjectedTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = inst_->CTLR2;
        tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
        tempreg.JEXTTRIG = (trigger != InjectedTrigger::SW);
        inst_->CTLR2 = tempreg.data;
    }

    void set_wdt_threshold(const uint16_t low,const uint16_t high){
        inst_->WDHTR = CLAMP(low, 0, get_max_value());
        inst_->WDLTR = CLAMP(high, 0, get_max_value());
    }


    void set_trigger(
        const RegularTrigger r_trigger, 
        const InjectedTrigger j_trigger
    ){
        set_regular_trigger(r_trigger);
        set_injected_trigger(j_trigger);
    }

    void sw_start_regular(const bool force = false){
        if(force) set_regular_trigger(RegularTrigger::SW);
        ADC_SoftwareStartConvCmd(inst_, true);
    }

    void sw_start_injected(const bool force = false){
        if(force) set_injected_trigger(InjectedTrigger::SW);
        ADC_SoftwareStartInjectedConvCmd(inst_, true);
    }

    [[nodiscard]] bool is_regular_idle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC);
    }

    [[nodiscard]] bool is_injected_idle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC);
    }

    [[nodiscard]] bool is_idle(){
        return (is_regular_idle() && is_injected_idle());
    }

    void enable_dma(const Enable en){
        ADC_DMACmd(inst_, en == EN);
    }

    uint16_t get_conv_result(){
        return inst_->RDATAR;
    }

protected:
    ADC_TypeDef * inst_;
    Callback callback_;

    bool right_align = true;

    int16_t cali_data;

    uint8_t regular_cnt = 0;
    uint8_t injected_cnt = 0;

    uint16_t regular_data_cache[16] = {0};

    AdcInjectedChannel injected_channels[4];


    uint32_t get_max_value() const {
        return ((1 << 12) - 1) << (right_align ? 0 : 4);
    }

    void set_regular_count(const uint8_t cnt){
        auto tempreg = std::bit_cast<CTLR1>(inst_->CTLR1);
        tempreg.DISCNUM = cnt;
        inst_->CTLR1 = std::bit_cast<uint32_t>(tempreg);
        regular_cnt = cnt;
    }

    void set_injected_count(const uint8_t cnt){
        ADC_InjectedSequencerLengthConfig(inst_, cnt);
        injected_cnt = cnt;
    }

    void set_regular_sample_time(const ChannelSelection nth,  const SampleCycles _sample_time){
        auto sample_time = _sample_time;
        uint8_t ch = std::bit_cast<uint8_t>(nth);
        uint8_t offset = ch % 10;
        offset *= 3;

        if(ch < 10){
            uint32_t tempreg = inst_->SAMPTR1;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            inst_->SAMPTR1 = tempreg;
        }else{
            uint32_t tempreg = inst_->SAMPTR2;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            inst_->SAMPTR2 = tempreg;
        }
    }

    void enable_singleshot(const Enable en){
        auto tempreg = std::bit_cast<CTLR1>(inst_->CTLR1);
        tempreg.DISCEN = en == EN;
        inst_->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_scan(const Enable en){
        auto tempreg = std::bit_cast<CTLR1>(inst_->CTLR1);
        tempreg.SCAN = en == EN;
        inst_->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_temp_vref(const Enable en){
        CTLR2 tempreg;
        tempreg.data = inst_->CTLR2;
        tempreg.TSVREFE = en == EN;
        inst_->CTLR2 = tempreg.data;
    }

    #if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
    friend void ::ADC1_2_IRQHandler(void);
    #endif

    __fast_inline void accept_jeoc_interrupt(){
        EXECUTE(callback_, AdcEvent::EndOfInjectedConversion);
    }

    __fast_inline void accept_eoc_interrupt(){
        EXECUTE(callback_, AdcEvent::EndOfConversion);
    }

    __fast_inline void accept_awd_interrupt(){
        EXECUTE(callback_, AdcEvent::AnalogWatchdog);
    }
};


}