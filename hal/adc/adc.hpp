#ifndef __MYADC_HPP__

#define __MYADC_HPP__

#include "types/range/range_t.hpp"
#include "sys/platform.h"

#include "hal/gpio/port.hpp"
#include "hal/nvic/nvic.hpp"

#include "regular_channel.hpp"
#include "injected_channel.hpp"
#include "adc_enums.h"
#include <initializer_list>

class AdcConcept{

};

struct AdcChannelConfig{
    using Channel = AdcChannel;
    using SampleCycles = AdcSampleCycles;

    Channel channel;
    SampleCycles sample_cycles;
};


class AdcOnChip:AdcConcept{
public:
    enum class Pga:uint8_t{
        X1, X4, X16, X64
    };

    enum class RegularTrigger:uint8_t{
        T1CC1, T1CC2, T1CC3, T2CC2, T3TRGO, T4CC4, EXTI11_T8TRGO, SW
    };

    enum class InjectedTrigger:uint8_t{
        T1TRGO, T1CC4, T2TRGO, T2CC1, T3CC4, T4TRGO, EXTI1515_T8CC4, SW
    };
// ADC_ExternalTrigInjecConv_T3_CC4
protected:
    ADC_TypeDef * instance;
    // ADC_TypeDef * instance2;
    struct CTLR1{
        union{
            struct{
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
            uint32_t data;
        };
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
    // static constexpr int u = sizeof(CTLR2);
public:
    AdcOnChip(ADC_TypeDef * _instance):instance(_instance){;}
};

class AdcCompanion:public AdcOnChip{
public:
    AdcCompanion(ADC_TypeDef * _instance):AdcOnChip(_instance){;}
};

class AdcPrimary: public AdcOnChip{
protected:

    using Mode = AdcMode;

    bool right_align = true;
    uint8_t regular_cnt = 0;
    uint8_t injected_cnt = 0;
    int16_t cali_data;

    using Channel = AdcChannel;
    using SampleTime = AdcSampleCycles;

    uint32_t getMaxValue() const {
        return ((1 << 12) - 1) << (right_align ? 0 : 4);
    }
    // void clearRegularQueue(){
    //     for(RegularChannel * & regular : regular_ptrs){
    //         regular = nullptr;
    //     }
    //     regular_cnt = 0;
    // }

    // void clearInjectedQueue(){
    //     for(InjectedChannel * & injected : injected_ptrs){
    //         injected = nullptr;
    //     }
    //     injected_cnt = 0;
    // }

    // void AddChannelToQueue(RegularChannel & regular_channel){
    //     if(regular_cnt >= 16) return;
    //     regular_ptrs[regular_cnt++] = &regular_channel;
    // }

    // void AddChannelToQueue(InjectedChannel & injected_channel){
    //     if(injected_cnt >= 4) return;
    //     injected_ptrs[injected_cnt++] = &injected_channel;
    // }
    void setRegularCount(const uint8_t cnt){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.DISCNUM = cnt;
        instance->CTLR1 = tempreg.data;
        regular_cnt = cnt;
    }

    void setInjectedCount(const uint8_t cnt){
        ADC_InjectedSequencerLengthConfig(instance, cnt);
        injected_cnt = cnt;
    }

    void setRegularSampleTime(const Channel channel,  const SampleTime _sample_time){
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

    void installPin(const Channel channel, const bool en = true){
        uint8_t ch_index = (uint8_t)channel;

        if(ch_index > 15) return;

        Port * gpio_port = nullptr;
        Pin gpio_pin = Pin::None;

        if(ch_index <= 7){
            gpio_port = &portA;
            gpio_pin = (Pin)(1 << ch_index);
        }else if(ch_index <= 9){
            gpio_port = &portB;
            gpio_pin = (Pin)(1 << (ch_index - 8));
        }else if(ch_index <= 15){
            gpio_port = &portC;
            gpio_pin = (Pin)(1 << (ch_index - 10));
        }

        Gpio & io = (*gpio_port)[gpio_pin];
        if(en)io.InAnalog();
        else io.InFloating();
    }

    void enableSingleshot(const bool en = true){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.DISCEN = en;
        instance->CTLR1 = tempreg.data;
    }

    void enableScan(const bool en = true){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.SCAN = en;
        instance->CTLR1 = tempreg.data;
    }

    void enableTempVref(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.TSVREFE = en;
        instance->CTLR2 = tempreg.data;
    }

public:
    AdcPrimary(ADC_TypeDef * _instance):AdcOnChip(_instance){;}

    void init(const std::initializer_list<AdcChannelConfig> & regular_list,
            const std::initializer_list<AdcChannelConfig> & injected_list, const Mode mode = Mode::Independent){

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        RCC_ADCCLKConfig(RCC_PCLK2_Div8);

        ADC_DeInit(instance);

        setMode(mode);
        ADC_Cmd(ADC1, ENABLE);

        ADC_BufferCmd(ADC1, DISABLE);

        ADC_ResetCalibration(ADC1);

        while(ADC_GetResetCalibrationStatus(ADC1));
        ADC_StartCalibration(ADC1);

        while(ADC_GetCalibrationStatus(ADC1));
        cali_data = Get_CalibrationValue(ADC1);

        ADC_BufferCmd(ADC1, ENABLE);

        bool temp_verf_activation = false;
    
        {
            setRegularCount(regular_list.size());
            uint8_t i = 0;
            for(auto config : regular_list){
                i++;
                ADC_RegularChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.sample_cycles);
                installPin(config.channel);

                temp_verf_activation |= (config.channel == Channel::TEMP || config.channel == Channel::VREF);

                if(i > 16) break;
            }
        }

        {
            setInjectedCount(injected_list.size());
            uint8_t i = 0;
            for(auto config : injected_list){
                i++;

                ADC_InjectedChannelConfig(instance,(uint8_t)config.channel,i,(uint8_t)config.sample_cycles);
                ADC_SetInjectedOffset(instance, ADC_InjectedChannel_1 + (ADC_InjectedChannel_2 - ADC_InjectedChannel_1) * (i-1),MAX(cali_data, 0)); // offset can`t be negative
                installPin(config.channel);

                temp_verf_activation |= (config.channel == Channel::TEMP || config.channel == Channel::VREF);

                if(i > 4) break;
            }
        }

        if(temp_verf_activation) enableTempVref();

        if(MAX(injected_list.size(), regular_list.size()) > 1) enableScan();
        else enableSingleshot();

    }

    void enableDma(const bool en = true){
        ADC_DMACmd(instance, en);
    }

    void enableIT(const NvicPriority & priority){

    }
    void setMode(const Mode mode){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.DUALMOD = (uint8_t)mode;
        instance->CTLR1 = tempreg.data;
    };

    void setPga(const Pga pga){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.PGA = (uint8_t)pga;
        instance->CTLR1 = tempreg.data;
    }

    void enableContinous(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.CONT = en;
        instance->CTLR2 = tempreg.data;
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
        instance->CTLR2 = tempreg.data;

        if(trigger != RegularTrigger::SW){
            ADC_ExternalTrigConvCmd(instance,ENABLE);
        }
    }

    void setInjectedTrigger(const InjectedTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
        instance->CTLR2 = tempreg.data;

        if(trigger != InjectedTrigger::SW){
            ADC_ExternalTrigInjectedConvCmd(instance, ENABLE);
            ADC_ExternalTrigConvCmd(instance,ENABLE);
        }else{
            ADC_ExternalTrigInjectedConvCmd(instance, DISABLE);
        }
    }

    void setWdtThreshold(const Range_t<int> & _threshold){
        auto threshold = _threshold.intersection(Rangei(0, (int)getMaxValue()));
        instance->WDHTR = threshold.end;
        instance->WDLTR = threshold.start;
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

    // void addDataCB(const uint16_t data){

    // }

    // RegularChannel getRegularChannel(const Channel channel, const SampleTime sample_time = SampleTime::T28_5){
    //     return RegularChannel(instance, channel, sample_time);
    // }

    // InjectedChannel getInjectedChannel(const Channel channel, const SampleTime sample_time = SampleTime::T28_5){
    //     return InjectedChannel(instance, channel, sample_time);
    // }

    virtual void refreshRegularData() = 0;
    virtual void refreshInjectedData() = 0;

    virtual uint16_t getRegularDataByRank(const uint8_t & rank) = 0;
    virtual uint16_t getInjectedDataByRank(const uint8_t & rank) = 0;
};

#endif