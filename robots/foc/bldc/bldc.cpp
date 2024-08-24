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
constexpr real_t dutyScale = real_t(0.27f);


constexpr int fsector(const real_t x, const real_t inv_step, int sectors){
    return int((x * inv_step) / sectors);
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

    en_gpio.outpp(0);
    slp_gpio.outpp(0);

    timer1.init(chopper_freq, TimerUtils::Mode::CenterAlignedUpTrig);
    timer1.enableArrSync();

    auto & pwm_u = timer1.oc(1); 
    auto & pwm_v = timer1.oc(2); 
    auto & pwm_w = timer1.oc(3); 
    timer1.oc(4).init(TimerUtils::OcMode::UpValid, false);
    timer1.oc(4).cnt() = 0;

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

    using AdcChannelEnum = AdcUtils::Channel;
    using AdcCycleEnum = AdcUtils::SampleCycles;

    adc1.init(
        {
            AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T28_5}
        },{
            AdcChannelConfig{AdcChannelEnum::CH1, AdcCycleEnum::T41_5},
            AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T41_5},
            AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T41_5},
            AdcChannelConfig{AdcChannelEnum::CH7, AdcCycleEnum::T41_5},
        }
    );

    adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1CC4);
    adc1.enableContinous();
    adc1.enableAutoInject();



    real_t rad = 0;
    real_t open_rad = 0;

    using Current = real_t;
    using Current3 = std::array<Current, 3>;
    using Current2 = std::array<Current, 2>;
    using Voltage = real_t;

    Current3 uvw_curr = {0,0,0};
    Current3 uvw_curr_drift = {0,0,0};

    Current2 ab_curr = {0,0};
    Current2 dq_curr = {0,0};

    Voltage bus_volt = 0;
    real_t adc_data[4];
    uint16_t adc_data_cache[4];

    auto data_to_curr = [](const real_t data) -> real_t{
        static constexpr real_t mul = real_t((3.3 * 9800 / 1000));
        return ((real_t(data) >> 4) * mul) >> 8;
        // return data;
    };

    auto uvw_to_ab = [](const Current3 & uvw) -> Current2{
        return {uvw[0] - ((uvw[1] + uvw[2]) >> 1), (uvw[2] - uvw[1]) * real_t(1.73 / 2)};
    };

    auto ab_to_dq = [](const Current2 & ab, const real_t rad) -> Current2{
        return {cos(rad) * ab[1] - sin(rad) * ab[0], sin(rad) * ab[1] + cos(rad) * ab[0]};
    };

    auto update_curr = [&](){
        // #define LPF(x,y) x = (x * 1023 + y) >> 10;
        // #define LPF(x,y) x = ((x* 31 + (y)) / 32);
        #define LPF(x,y) x = (((x >> 4) * 31 + (y >> 4))) >> 1;
        // #define LPF(x,y) x = (y * 64) >> 6;
        adc_data_cache[0] = int(ADC1->IDATAR1);
        adc_data_cache[1] = int(ADC1->IDATAR2);
        adc_data_cache[2] = int(ADC1->IDATAR3);

        static real_t temp_adc_data[4];
        LPF(temp_adc_data[0], int(ADC1->IDATAR1));
        LPF(temp_adc_data[1], int(ADC1->IDATAR2));
        LPF(temp_adc_data[2], int(ADC1->IDATAR3));
        LPF(temp_adc_data[3], int(ADC1->IDATAR4));

        LPF(adc_data[0], temp_adc_data[0]);
        LPF(adc_data[1], temp_adc_data[1]);
        LPF(adc_data[2], temp_adc_data[2]);
        LPF(adc_data[3], temp_adc_data[3]);

        auto data_to_volt = [](const real_t data) -> real_t{
            return (data * real_t(33.3/4) >> 10);
        };

        for(size_t i = 0; i < 3; i++){
            uvw_curr[i] = data_to_curr(adc_data[i]) - uvw_curr_drift[i];
        }

        ab_curr = uvw_to_ab(uvw_curr);


        bus_volt = data_to_volt(adc_data[3]);
    };


    auto cb = [&](){

        update_curr();

        odo.update();
        auto pos = ma730.getLapPosition();
        rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.7)  - frac(pos * 7) * real_t(TAU);
        real_t open_pos = t / 10;
        open_rad = frac(open_pos * 7) * real_t(TAU);
        setDQDuty(0, real_t(0.01), rad);
        // auto temp_dq_curr = ab_to_dq(ab_curr, rad);
        dq_curr = ab_to_dq(ab_curr, rad);
        // LPF(dq_curr[0], temp_dq_curr[0])
        // LPF(dq_curr[1], temp_dq_curr[1])


    };


    adc1.bindCb(AdcUtils::IT::JEOC, cb);


    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    for(size_t i = 0; i < 40000; ++i){
        update_curr();
    }
    std::swap(uvw_curr, uvw_curr_drift);

    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

    // DEBUG_PRINTLN(uvw_curr_drift);

    en_gpio = true;
    slp_gpio = true;
    while(true){
        // auto pos = ma730.getLapPosition();

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;

        // auto _t = real_t(0);

        // DEBUG_PRINTLN(uvw_curr);

        // CanMsg msg = {0x11, uint8_t(0x57)};
        // if(can1.pending() == 0) can1.write(msg);
        // , real_t(pwm_v), real_t(pwm_w), std::dec, data[0]>>12, data[1] >>12, data[2]>>12);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(rad, open_rad, odo.getPosition(), std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], bus_volt);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, adc_data_cache[0], adc_data_cache[1], adc_data_cache[2], (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3);
        DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1], dq_curr[0], dq_curr[1]);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3);

        // cb();
        // delay(20);
        // DEBUG_PRINTLN(spi1.cs_port.isIndexValid(0), spi1.cs_port.isIndexValid(1), spi1.cs_port.isIndexValid(2))
        // bmi.check();
        // delay(20);
    }
}