#include "../sys/core/system.hpp"
#include "../sys/kernel/clock.h"

#include "../types/vector3/vector3_t.hpp"
#include "../types/quat/Quat_t.hpp"


#include "../hal/timer/instance/timer_hw.hpp"
#include "../hal/adc/adcs/adc1.hpp"
#include "../hal/bus/can/can.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "hal/bus/spi/spihw.hpp"


SpiDrv ma730_drv{spi1, 0};
MA730 ma730{ma730_drv};

using Sys::t;
// constexpr int pwmFreq = 67000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -14;
// constexpr float dutyScale = 0.55f;
// constexpr float mk = 0.17;

// constexpr int pwmFreq = 67000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -28;
// constexpr float dutyScale = 0.6f;
// constexpr float mk = 0.42f;

// constexpr int pwmFreq = 63000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -24;
// constexpr float dutyScale = 0.3f;
// constexpr float mk = 0.38f;

// constexpr int pwmFreq = 32000;
// constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
// constexpr float sample_ticks = -15;
// constexpr float dutyScale = 0.3f;
// constexpr float mk = 0.3f;

constexpr int pwmFreq = 73000;
constexpr auto adc_sample_cycles = ADC_SampleTime_28Cycles5;
constexpr float sample_ticks = -10.5;
constexpr real_t dutyScale = real_t(0.17f);


constexpr int fsector(const real_t x, const real_t inv_step, int sectors){
    return int((x * inv_step) / sectors);
}


void ADC1_Init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC3;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    // ADC_InitStructure.ADC_Pga = ADC_Pga_4;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, adc_sample_cycles);
    
    ADC_InjectedSequencerLengthConfig(ADC1, 4);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_1,1,adc_sample_cycles);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_4,2,adc_sample_cycles);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_5,3,adc_sample_cycles);
    ADC_InjectedChannelConfig(ADC1,ADC_Channel_7,4,adc_sample_cycles);

    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);
    // ADC_ExternalTrigInjectedConvEdgeConfig(ADC1,adcexternaltriginjeced);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1, DISABLE);
    
    // ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
    // ADC_ITConfig(ADC1,ADC_IT_EOC, ENABLE);
    ADC_ITConfig(ADC1,ADC_IT_JEOC,ENABLE);//ENABLE INJECTED INTERRUPT
    
    ADC_AutoInjectedConvCmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);

    ADC_BufferCmd(ADC1, DISABLE); //disable buffer
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    // Calibrattion_Val = Get_CalibrationValue(ADC1);

    ADC_BufferCmd(ADC1, ENABLE); //enable buffer

}



void setUVWDuty(const real_t uDutyTarget,const real_t vDutyTarget,const real_t wDutyTarget){
    timer1.oc(1) = uDutyTarget;
    timer1.oc(2) = vDutyTarget;
    timer1.oc(3) = wDutyTarget;
}



void setDQDuty(const real_t dDutyTarget,const real_t qDutyTarget,const real_t radTarget){

    auto modu_sect = (int(radTarget / real_t(TAU / 6)) % 6) + 1;
    auto sixtant_theta = fmod(radTarget, real_t(TAU / 6));
    

    real_t ta = std::sin(sixtant_theta) * real_t(dutyScale);
    real_t tb = std::sin(real_t(TAU / 6) - sixtant_theta) * real_t(dutyScale);
    
    real_t t0 = (real_t(1) - ta - tb) / 2;
    real_t t1 = (real_t(1) + ((modu_sect % 2 == 0 )? (tb - ta) : (ta - tb))) / 2;
    real_t t2 = (real_t(1) + ta + tb) / 2;

    switch (modu_sect){
        case 1:
            setUVWDuty(t2, t1, t0);
            break;
        case 2:
            setUVWDuty(t1, t2, t0);
            break;
        case 3:
            setUVWDuty(t0, t2, t1);
            break;
        case 4:
            setUVWDuty(t0, t1, t2);
            break;
        case 5:
            setUVWDuty(t1, t0, t2);
            break;
        case 6:
            setUVWDuty(t2, t0, t1);
            break;
        default:
            break;
    }
}


// void processCurrentSensing(){

//     const int16_t currDataThree = currData[0];
//     const int16_t currDataTwo = currData[1];
//     const int16_t currDataOne = currData[2];
//     const int16_t currDataZero = currData[3];

//     int16_t currDataFirst = currDataOne - currDataZero;
//     int16_t currDataSecond = currDataTwo - currDataOne;
//     int16_t currDataThird = currDataThree - currDataTwo;

    // int16_t uCurrData, vCurrData, wCurrData;

    // switch(trig_sect){
    
    // case 1:
    //     uCurrData = currDataThird;
    //     vCurrData = currDataSecond;
    //     wCurrData = currDataFirst;
    //     break;

    // case 2:
    //     uCurrData = currDataSecond;
    //     vCurrData = currDataThird;
    //     wCurrData = currDataFirst;
    //     break;

    // case 3:
    //     uCurrData = currDataFirst;
    //     vCurrData = currDataThird;
    //     wCurrData = currDataSecond;
    //     break;

    // case 4:
    //     uCurrData = currDataFirst;
    //     vCurrData = currDataSecond;
    //     wCurrData = currDataThird;
    //     break;

    // case 5:
    //     uCurrData = currDataSecond;
    //     vCurrData = currDataFirst;
    //     wCurrData = currDataThird;
    //     break;

    // case 6:
    //     uCurrData = currDataThird;
    //     vCurrData = currDataFirst;
    //     wCurrData = currDataSecond;
    //     break;

    // default:
    //     uCurrData = 0;
    //     vCurrData = 0;
    //     wCurrData = 0;
    //     break;
    // }

    // uCurr = currData2curr(uCurrData);
    // vCurr = currData2curr(vCurrData);
    // wCurr = currData2curr(wCurrData);
//     alphaCurr = (uCurr - std::mean(vCurr, wCurr)) * real_t(2.0 / 3.0);
//     betaCurr = (vCurr - wCurr) * real_t(SQRT3/3);
// }


static constexpr uint foc_freq = 32768;
static constexpr uint chopper_freq = foc_freq * 2;


int bldc_main(){
    DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Blocking);

    auto & en_gpio = portA[11];
    auto & slp_gpio = portA[12];

    en_gpio.outpp(1);
    slp_gpio.outpp(1);

    timer1.init(chopper_freq, TimerUtils::Mode::CenterAlignedUpTrig);
    timer1.enableArrSync();

    auto & pwm_u = timer1.oc(1); 
    auto & pwm_v = timer1.oc(2); 
    auto & pwm_w = timer1.oc(3); 
    timer1.oc(4).init(TimerUtils::OcMode::UpValid, false);
    timer1.oc(4) = real_t(0.001);

    pwm_u.init();
    pwm_v.init();
    pwm_w.init();

    spi1.init(18_MHz);
    spi1.bindCsPin(portA[15], 2);
    spi1.bindCsPin(portA[0], 0);
    can1.init(1_MHz);


    MA730 ma730{spi1, 2};
    ma730.init();

    Odometer odo{ma730};
    odo.init();

    // ma730.setDirection(false);

    // using AdcChannelEnum = AdcUtils::Channel;
    // using AdcCycleEnum = AdcUtils::SampleCycles;

    // adc1.init(
    //     {
    //         AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T239_5}
    //     },{
    //         AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T239_5},
    //         AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T239_5},
    //         AdcChannelConfig{AdcChannelEnum::CH1, AdcCycleEnum::T239_5},
    //         AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T239_5},
    //     }
    // );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1CC4);
    // adc1.enableContinous();
    // adc1.enableAutoInject();
    ADC1_Init();

    real_t data[4];

    #define LPF(x,y,a) x = real_t(x * a + y * (1-a));

    real_t rad = 0;
    real_t open_rad = 0;
    auto cb = [&](){
        static constexpr auto alaph = real_t(0.99);
        LPF(data[0], uint16_t(ADC1->IDATAR1), alaph);
        LPF(data[1], uint16_t(ADC1->IDATAR2), alaph);
        LPF(data[2], uint16_t(ADC1->IDATAR3), alaph);
        LPF(data[3], uint16_t(ADC1->IDATAR4), alaph);

        odo.update();
        auto pos = ma730.getLapPosition();
        rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.7)  - frac(pos * 7) * real_t(TAU);
        real_t open_pos = t / 10;
        open_rad = frac(open_pos * 7) * real_t(TAU);
        setDQDuty(0, real_t(0.01), rad);
        // static constexpr auto scale = real_t(0.07);
        // static constexpr auto offset = real_t(0.01);
        // pwm_u = scale * sin(_t) + scale + offset;
        // pwm_v = scale * sin(_t + real_t(PI * 2 / 3)) + scale + offset;
        // pwm_w = scale * sin(_t - real_t(PI * 2 / 3)) + scale + offset;
    };
    adc1.bindCb(AdcUtils::IT::JEOC, cb);
    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();
    while(true){
        // auto pos = ma730.getLapPosition();

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;

        // auto _t = real_t(0);

        


        // CanMsg msg = {0x11, uint8_t(0x57)};
        // if(can1.pending() == 0) can1.write(msg);
        // , real_t(pwm_v), real_t(pwm_w), std::dec, data[0]>>12, data[1] >>12, data[2]>>12);
        if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(rad, open_rad, odo.getPosition(), std::setprecision(3), std::dec, data[0]>>10, data[1] >>10, data[2]>>10, (data[3] * real_t(33.3/4) >> 10), ADC1->IDATAR4);
        // cb();
        // delay(20);
        // DEBUG_PRINTLN(spi1.cs_port.isIndexValid(0), spi1.cs_port.isIndexValid(1), spi1.cs_port.isIndexValid(2))
        // bmi.check();
        // delay(20);
    }
}