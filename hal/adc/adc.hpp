#pragma once

#include "../sys/core/platform.h"

#include "../hal/gpio/port.hpp"
#include "../hal/nvic/nvic.hpp"
#include "../hal/dma/dma.hpp"

#include "regular_channel.hpp"
#include "injected_channel.hpp"
#include "adc_utils.hpp"
#include <initializer_list>

#if defined(HAVE_ADC1) || defined(HAVE_ADC2)
extern "C"{
__interrupt void ADC1_2_IRQHandler(void);
}
#endif

class AdcConcept{
protected:
public:
    using Callback = std::function<void(void)>;
};

struct AdcChannelConfig{
public:
    using Channel = AdcUtils::Channel;
    using SampleCycles = AdcUtils::SampleCycles;

    Channel channel;
    SampleCycles cycles = SampleCycles::T41_5;
};


class AdcOnChip:AdcConcept{
public:

    using Pga = AdcUtils::Pga;
    using RegularTrigger = AdcUtils::RegularTrigger;
    using InjectedTrigger = AdcUtils::InjectedTrigger;

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

class AdcCompanion:public AdcOnChip{
public:
    AdcCompanion(ADC_TypeDef * _instance):AdcOnChip(_instance){;}
};

class AdcPrimary: public AdcOnChip{
protected:
    Callback jeoc_cb;
    Callback eoc_cb;
    Callback awd_cb;

    using Channel = AdcUtils::Channel;
    using SampleCycles = AdcUtils::SampleCycles;
    using Mode = AdcUtils::Mode;
    using IT = AdcUtils::IT;
    using Callback = AdcUtils::Callback;

    bool right_align = true;

    int16_t cali_data;

    uint8_t regular_cnt = 0;
    uint8_t injected_cnt = 0;

    uint16_t regular_data_cache[16] = {0};

    // RegularChannel regular_channels[16];
    InjectedChannel injected_channels[4];


    uint32_t getMaxValue() const {
        return ((1 << 12) - 1) << (right_align ? 0 : 4);
    }

    void setRegularCount(const uint8_t cnt){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DISCNUM = cnt;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
        regular_cnt = cnt;
    }

    void setInjectedCount(const uint8_t cnt){
        ADC_InjectedSequencerLengthConfig(instance, cnt);
        injected_cnt = cnt;
    }

    void setRegularSampleTime(const Channel channel,  const SampleCycles _sample_time){
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

    void enableSingleshot(const bool en = true){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DISCEN = en;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enableScan(const bool en = true){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.SCAN = en;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enableTempVref(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.TSVREFE = en;
        instance->CTLR2 = tempreg.data;
    }

    friend void ADC1_2_IRQHandler(void);
public:
    AdcPrimary(ADC_TypeDef * _instance):AdcOnChip(_instance),
        injected_channels{
            InjectedChannel(instance, Channel::VREF, 1),
            InjectedChannel(instance, Channel::VREF, 2),
            InjectedChannel(instance, Channel::VREF, 3),
            InjectedChannel(instance, Channel::VREF, 4)
        }{;}

    void init(const std::initializer_list<AdcChannelConfig> & regular_list,
            const std::initializer_list<AdcChannelConfig> & injected_list, 
            const Mode mode = Mode::Independent);

    auto & inj(const uint8_t index){
        if(index == 0 or index > 4) CREATE_FAULT;
        return injected_channels[index - 1];
    }

    void bindCb(const IT it,Callback && cb);

    void enableIT(const IT it, const NvicPriority & priority){
        ADC_ITConfig(instance, (uint16_t)it, true);
        priority.enable(ADC_IRQn);
    }

    void setMode(const Mode mode){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.DUALMOD = (uint8_t)mode;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    };

    void setPga(const Pga pga){
        auto tempreg = std::bit_cast<CTLR1>(instance->CTLR1);
        tempreg.PGA = (uint8_t)pga;
        instance->CTLR1 = std::bit_cast<uint32_t>(tempreg);
    }

    void enableContinous(const bool en = true){
        auto tempreg = std::bit_cast<CTLR2>(instance->CTLR2);
        tempreg.CONT = en;
        instance->CTLR2 = std::bit_cast<uint32_t>(tempreg);
    }

    void enableAutoInject(const bool en = true){
        ADC_AutoInjectedConvCmd(instance, en);
    }

    void enableRightAlign(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.ALIGN = !en;
        instance->CTLR2 = tempreg.data;
        right_align = en;
    }

    void setRegularTrigger(const RegularTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.EXTSEL = static_cast<uint8_t>(trigger);
        tempreg.EXTTRIG = (trigger != RegularTrigger::SW);
        instance->CTLR2 = tempreg.data;
    }

    void setInjectedTrigger(const InjectedTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
        tempreg.JEXTTRIG = (trigger != InjectedTrigger::SW);
        instance->CTLR2 = tempreg.data;
    }

    void setWdtThreshold(const int low,const int high){
        instance->WDHTR = CLAMP(low, 0, getMaxValue());
        instance->WDLTR = CLAMP(high, 0, getMaxValue());
    }

    void bindWdtIt(Callback && cb){
        //TODO
    }

    void setTrigger(const RegularTrigger _rtrigger, const InjectedTrigger _jtrigger){
        setRegularTrigger(_rtrigger);
        setInjectedTrigger(_jtrigger);
    }

    void swStartRegular(const bool force = false){
        if(force) setRegularTrigger(RegularTrigger::SW);
        ADC_SoftwareStartConvCmd(instance, true);
    }

    void swStartInjected(const bool force = false){
        if(force) setInjectedTrigger(InjectedTrigger::SW);
        ADC_SoftwareStartInjectedConvCmd(instance, true);
    }

    bool isRegularIdle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC);
    }

    bool isInjectedIdle(){
        return ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC);
    }

    bool isIdle(){
        return (isRegularIdle() && isInjectedIdle());
    }

    void enableDma(const bool en = true){
        ADC_DMACmd(instance, en);
    }

    uint16_t getConvResult(){
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

    virtual void refreshRegularData() = 0;
    virtual void refreshInjectedData() = 0;

    virtual uint16_t getRegularDataByRank(const uint8_t & rank) = 0;
    virtual uint16_t getInjectedDataByRank(const uint8_t & rank) = 0;
};
