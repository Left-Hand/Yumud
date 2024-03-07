#ifndef __MYADC_HPP__

#define __MYADC_HPP__

#include "src/platform.h"
#include "regular_channel.hpp"
#include "injected_channel.hpp"
#include <initializer_list>

class Adc{
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
    Adc(ADC_TypeDef * _instance):instance(_instance){;}
};

class AdcCompanion:public Adc{

};

class AdcPrimary: public Adc{
protected:
    RegularChannel * regular_ptrs[16];
    uint8_t disc_index = 0;
public:
    AdcPrimary(ADC_TypeDef * _instance):Adc(_instance){;}
    void cali(){
        ADC_BufferCmd(ADC1, DISABLE);

        ADC_ResetCalibration(ADC1);

        while(ADC_GetResetCalibrationStatus(ADC1));
        ADC_StartCalibration(ADC1);

        while(ADC_GetCalibrationStatus(ADC1));
        cali_data = Get_CalibrationValue(ADC1);

        ADC_BufferCmd(ADC1, ENABLE);
    }

    // void init(std::initializer_list<const RegularChannel &> _regulars, std::initializer_list<const InjectedChannel &> _injecteds){
    //     ADC_InitTypeDef  ADC_InitStructure = {0};

    //     RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    //     RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    //     ADC_DeInit(ADC1);
    //     ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    //     ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    //     ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    //     ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    //     ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    //     ADC_InitStructure.ADC_NbrOfChannel = 2;
    //     ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    //     ADC_Init(ADC1, &ADC_InitStructure);

    //     // uint8_t cnt = 0;
    //     // for(auto & it = regulars.begin(); it != regulars.end(); it++){
            
    //     // }
    //     ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_28Cycles5);
    //     // ADC_InjectedChannelConfig
    //     ADC_SetInjectedOffset(ADC1,ADC_InjectedChannel_1,0);
    // }

    void initDma(){
        ADC_DMACmd(ADC1, ENABLE);
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

    void enableLeftAlign(const bool en = true){
        CTLR2 tempreg;
        tempreg.data = instance->CTLR2;
        tempreg.ALIGN= en;
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

    void startConv(){
        ADC_Cmd(ADC1, ENABLE);
    }
};

#endif