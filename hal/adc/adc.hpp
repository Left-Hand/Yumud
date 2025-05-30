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

class AdcConcept{
protected:
public:
    using Callback = std::function<void()>;
};

struct AdcChannelConfig{
public:
    using ChannelIndex = AdcChannelIndex;
    using SampleCycles = AdcSampleCycles;

    ChannelIndex channel;
    SampleCycles cycles;
};


class AdcOnChip:AdcConcept{
public:

    using Pga = AdcPga;
    using RegularTrigger = AdcRegularTrigger;
    using InjectedTrigger = AdcInjectedTrigger;

protected:
    ADC_TypeDef * instance;
    using AdcConcept::Callback;

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

public:
    AdcOnChip(ADC_TypeDef * _instance):instance(_instance){;}
};



class AdcPrimary: public AdcOnChip{
protected:
    Callback jeoc_cb;
    Callback eoc_cb;
    Callback awd_cb;

    using ChannelIndex = AdcChannelIndex;
    using SampleCycles = AdcSampleCycles;
    using Mode = AdcMode;
    using IT = AdcIT;

    bool right_align = true;

    int16_t cali_data;

    uint8_t regular_cnt = 0;
    uint8_t injected_cnt = 0;

    uint16_t regular_data_cache[16] = {0};

    // RegularChannel regular_channels[16];
    AdcInjectedChannel injected_channels[4];


    uint32_t get_max_value() const {
        return ((1 << 12) - 1) << (right_align ? 0 : 4);
    }

    void set_regular_count(const uint8_t cnt){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DISCNUM = cnt;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
        regular_cnt = cnt;
    }

    void set_injected_count(const uint8_t cnt){
        ADC_InjectedSequencerLengthConfig(instance, cnt);
        injected_cnt = cnt;
    }

    void set_regular_sample_time(const ChannelIndex channel,  const SampleCycles _sample_time){
        auto sample_time = _sample_time;
        uint8_t ch = (uint8_t)channel;
        uint8_t offset = ch % 10;
        offset *= 3;

        if(ch < 10){
            uint32_t tempreg = instance->SAMPTR1;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            instance->SAMPTR1 = tempreg;
        }else{
            uint32_t tempreg = instance->SAMPTR2;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            instance->SAMPTR2 = tempreg;
        }
    }

    void enable_singleshot(const Enable en){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DISCEN = en == EN;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_scan(const Enable en){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.SCAN = en == EN;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_temp_vref(const Enable en){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.TSVREFE = en == EN;
        instance->CTLR2 = tempreg.data;
    }

    #if defined(ENABLE_ADC1) || defined(ENABLE_ADC2)
    friend void ::ADC1_2_IRQHandler(void);
    #endif

    __fast_inline void on_jeoc_interrupt(){
        EXECUTE(jeoc_cb);
    }

    __fast_inline void on_eoc_interrupt(){
        EXECUTE(eoc_cb);
    }

    __fast_inline void on_awd_interrupt(){
        EXECUTE(awd_cb);
    }
public:
    AdcPrimary(ADC_TypeDef * _instance):AdcOnChip(_instance),
        injected_channels{
            AdcInjectedChannel(instance, AdcChannelIndex::VREF, 1),
            AdcInjectedChannel(instance, AdcChannelIndex::VREF, 2),
            AdcInjectedChannel(instance, AdcChannelIndex::VREF, 3),
            AdcInjectedChannel(instance, AdcChannelIndex::VREF, 4)
        }{;}

    void init(const std::initializer_list<AdcChannelConfig> & regular_list,
            const std::initializer_list<AdcChannelConfig> & injected_list, 
            const Mode mode = Mode::Independent);

    AdcInjectedChannel & inj(const size_t index);

    void bind_cb(const IT it,auto && cb){
        switch(it){
            case IT::JEOC:
                jeoc_cb = std::forward<decltype(cb)>(cb);
                break;
            case IT::EOC:
                eoc_cb = std::forward<decltype(cb)>(cb);
                break;
            case IT::AWD:
                awd_cb = std::forward<decltype(cb)>(cb);
                break;
            default:
                break;
        }
    }

    void enable_it(const IT it, const NvicPriority priority, const Enable en = EN){
        ADC_ITConfig(instance, std::bit_cast<uint16_t>(it), en == EN);
        priority.enable(ADC_IRQn);
    }

    void attach(const IT it, const NvicPriority priority, auto && cb, const Enable en = EN){
        bind_cb(it, std::forward<decltype(cb)>(cb));
        enable_it(it, priority, en);
    }

    void attach(const IT it, const NvicPriority priority, std::nullptr_t cb){
        attach(it, priority, nullptr, DISEN);
    }

    void set_mode(const Mode mode){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DUALMOD = (uint8_t)mode;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    };

    void set_pga(const Pga pga){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.PGA = (uint8_t)pga;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_continous(const Enable en = EN){
        auto tempreg = std::bit_cast<CTLR2>(instance->CTLR2);
        tempreg.CONT = en == EN;
        instance->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    }

    void enable_auto_inject(const Enable en = EN){
        ADC_AutoInjectedConvCmd(instance, en == EN);
    }

    void enable_right_align(const Enable en = EN){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.ALIGN = en == DISEN;
        instance->CTLR2 = tempreg.data;
        right_align = en == EN;
    }

    void set_regular_trigger(const RegularTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.EXTSEL = static_cast<uint8_t>(trigger);
        tempreg.EXTTRIG = (trigger != RegularTrigger::SW);
        instance->CTLR2 = tempreg.data;
    }

    void set_injected_trigger(const InjectedTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
        tempreg.JEXTTRIG = (trigger != InjectedTrigger::SW);
        instance->CTLR2 = tempreg.data;
    }

    void set_wdt_threshold(const int low,const int high){
        instance->WDHTR = CLAMP(low, 0, get_max_value());
        instance->WDLTR = CLAMP(high, 0, get_max_value());
    }

    void bind_wdt_it(Callback && cb){
        //TODO
    }

    void set_trigger(const RegularTrigger _rtrigger, const InjectedTrigger _jtrigger){
        set_regular_trigger(_rtrigger);
        set_injected_trigger(_jtrigger);
    }

    void sw_start_regular(const bool force = false){
        if(force) set_regular_trigger(RegularTrigger::SW);
        ADC_SoftwareStartConvCmd(instance, true);
    }

    void sw_start_injected(const bool force = false){
        if(force) set_injected_trigger(InjectedTrigger::SW);
        ADC_SoftwareStartInjectedConvCmd(instance, true);
    }

    bool is_regular_idle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC);
    }

    bool is_injected_idle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC);
    }

    bool is_idle(){
        return (is_regular_idle() && is_injected_idle());
    }

    void enable_dma(const Enable en = EN){
        ADC_DMACmd(instance, en == EN);
    }

    uint16_t get_conv_result(){
        return instance->RDATAR;
    }

    // void addDataCB(const uint16_t data){

    // }

    // AdcChannelOnChip & ch(int8_t channel){
    //     if(channel > 0){
    //         return regular_channels[channel-1];
    //     }
    //     return 
    // }
    // RegularChannel getRegularChannel(const Channel channel, const SampleCycles sample_time = SampleCycles::T28_5){
    //     return RegularChannel(instance, channel, sample_time);
    // }

    // InjectedChannel getInjectedChannel(const Channel channel, const SampleCycles sample_time = SampleCycles::T28_5){
    //     return InjectedChannel(instance, channel, sample_time);
    // }

    virtual void refresh_regular_data() = 0;
    virtual void refresh_injected_data() = 0;

    virtual uint16_t get_regular_data_by_rank(const uint8_t rank) = 0;
    virtual uint16_t get_injected_data_by_rank(const uint8_t rank) = 0;
};


class AdcCompanion:public AdcOnChip{
public:
    AdcCompanion(ADC_TypeDef * _instance):AdcOnChip(_instance){;}
};

}