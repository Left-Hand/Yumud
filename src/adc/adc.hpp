#ifndef __MYADC_HPP__

#define __MYADC_HPP__

#include "src/platform.h"
#include "regular_channel.hpp"
#include "injected_channel.hpp"
#include "adc_enums.h"
#include <initializer_list>

class AdcConcept{

};

class AdcHw:AdcConcept{
public:
    enum class Pga:uint8_t{
        X1, X4, X16, X64
    };

    enum class RegularTrigger:uint8_t{
        T1CC1, T1CC2, T1CC3, T2CC2, T3TRGO, T4CC4, EXTI11_T8TRGO, SW
    };

    enum class InjectTrigger:uint8_t{
        T1TRGO, T1CC4, T2TRGO, T2CC1, T3CC4, T4TRGO, EXTI1515_T8CC4, SW
    };

protected:
    ADC_TypeDef * instance;
    int16_t cali_data;

    struct CTLR1{
        union{
            struct{
                uint32_t AWDCH:4;
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

public:
    AdcHw(ADC_TypeDef * _instance):instance(_instance){;}
};

class AdcCompanion:public AdcHw{
public:
    AdcCompanion(ADC_TypeDef * _instance):AdcHw(_instance){;}
};

class AdcPrimary: public AdcHw{
protected:
    RegularChannel * regular_ptrs[16] = {nullptr};
    InjectedChannel * injected_ptrs[4] = {nullptr};

    uint8_t regular_cnt = 0;
    uint8_t injected_cnt = 0;

    uint8_t disc_index = 0;

    using Channel = AdcChannels;
    using SampleTime = AdcSampleTimes;
public:
    AdcPrimary(ADC_TypeDef * _instance):AdcHw(_instance){;}
    void cali(){
        ADC_BufferCmd(ADC1, DISABLE);

        ADC_ResetCalibration(ADC1);

        while(ADC_GetResetCalibrationStatus(ADC1));
        ADC_StartCalibration(ADC1);

        while(ADC_GetCalibrationStatus(ADC1));
        cali_data = Get_CalibrationValue(ADC1);

        ADC_BufferCmd(ADC1, ENABLE);
    }

    void clearRegularQueue(){
        for(RegularChannel * & regular : regular_ptrs){
            regular = nullptr;
        }
        regular_cnt = 0;
    }

    void clearInjectedQueue(){
        for(InjectedChannel * & injected : injected_ptrs){
            injected = nullptr;
        }
        injected_cnt = 0;
    }

    void AddChannelToQueue(RegularChannel & regular_channel){
        if(regular_cnt >= 16) return;
        regular_ptrs[regular_cnt++] = &regular_channel;
    }

    void AddChannelToQueue(InjectedChannel & injected_channel){
        if(injected_cnt >= 4) return;
        injected_ptrs[injected_cnt++] = &injected_channel;
    }

    void init(){
        ADC_InitTypeDef  ADC_InitStructure = {0};

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        RCC_ADCCLKConfig(RCC_PCLK2_Div8);

        ADC_DeInit(ADC1);

        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ScanConvMode = DISABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = regular_cnt;
        ADC_InitStructure.ADC_Pga = ADC_Pga_1;
        ADC_Init(ADC1, &ADC_InitStructure);

        for(uint8_t i = 0; i < regular_cnt; i++){
            RegularChannel & cha = *regular_ptrs[i];
            uint8_t ch_index = (uint8_t)cha.channel;
            ADC_RegularChannelConfig(ADC1, ch_index, i+1, (uint8_t)cha.sample_time);
            if(ch_index >= (uint8_t)Channel::TEMP){
                enableTempVref();
            }
            cha.installToPin();
        }

        ADC_InjectedSequencerLengthConfig(ADC1, injected_cnt);
        for(uint8_t i = 0; i < injected_cnt; i++){
            InjectedChannel & cha = *injected_ptrs[i];
            ADC_InjectedChannelConfig(ADC1, (uint8_t)cha.channel, i+1, (uint8_t)cha.sample_time);
            ADC_SetInjectedOffset(ADC1,ADC_InjectedChannel_1, MAX(cali_data, 0)); // offset can`t be negative
            cha.installToPin();
        }

        ADC_Cmd(instance, ENABLE);
    }

    void enableDma(const bool en = true){
        ADC_DMACmd(ADC1, en);
    }

    void setPga(const Pga pga){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.PGA = (uint8_t)pga;
        instance->CTLR1 = tempreg.data;
    }

    void enableCont(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.CONT = en;
        instance->CTLR2 = tempreg.data;
    }

    void enableScan(const bool en = true){
        CTLR1 tempreg;
        tempreg.data = instance->CTLR1;
        tempreg.SCAN = en;
        instance->CTLR1 = tempreg.data;
    }

    void enableRightAlign(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.ALIGN = !en;
        instance->CTLR2 = tempreg.data;
    }

    void enableTempVref(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.TSVREFE = en;
        instance->CTLR2 = tempreg.data;
    }

    void setRegularTrigger(const RegularTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.EXTSEL = static_cast<uint8_t>(trigger);
        instance->CTLR2 = tempreg.data;
    }

    void setInjectTrigger(const InjectTrigger trigger){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.JEXTSEL = static_cast<uint8_t>(trigger);
        instance->CTLR2 = tempreg.data;
    }

    void setHighThr(uint16_t thr){
        if(thr > 4096) thr >>= 4;
        instance->WDHTR = thr;
    }

    void setLowThr(uint16_t thr){
        if(thr > 4096) thr >>= 4;
        instance->WDLTR = thr;
    }

    void start(){
        ADC_Cmd(ADC1, ENABLE);
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

    void addDataCB(const uint16_t data){

    }

    RegularChannel getRegularChannel(const Channel channel, const SampleTime sample_time = SampleTime::T28_5){
        return RegularChannel(instance, channel, sample_time);
    }

    InjectedChannel getInjectedChannel(const Channel channel, const SampleTime sample_time = SampleTime::T28_5){
        return InjectedChannel(instance, channel, sample_time);
    }
};

#endif