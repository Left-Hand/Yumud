#include "../sys/core/system.hpp"

#include "../types/vector3/vector3_t.hpp"
#include "../types/quat/Quat_t.hpp"


#include "../hal/timer/instance/timer_hw.hpp"
#include "../hal/adc/adcs/adc1.hpp"
#include "../hal/bus/can/can.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "drivers/Actuator/Driver/MP6540/mp6540.hpp"
#include "drivers/Actuator/SVPWM/svpwm.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"




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


// scexpr uint foc_freq = 32768;
// scexpr uint chopper_freq = 32768;


scexpr uint chopper_freq = 32768;

using Current = real_t;
using Current3 = std::array<Current, 3>;
using Current2 = std::array<Current, 2>;
using Voltage = real_t;


__inline auto data_to_curr(const real_t data) -> real_t{
    scexpr int res = 1000;
    scexpr real_t mul = real_t((3.3 * 9800 / res));
    return ((real_t(data) >> 4) * mul) >> 8;
};

__inline auto uvw_to_ab(const Current3 & uvw) -> Current2{
    return {uvw[0] - ((uvw[1] + uvw[2]) >> 1), (uvw[2] - uvw[1]) * real_t(1.73 / 2)};
};

__inline auto ab_to_dq(const Current2 & ab, const real_t rad) -> Current2{
    return {cos(rad) * ab[1] - sin(rad) * ab[0], sin(rad) * ab[1] + cos(rad) * ab[0]};
};



int bldc_main(){
    DEBUGGER.init(576000, CommMethod::Blocking);

    auto & en_gpio = portA[11];
    auto & slp_gpio = portA[12];

    en_gpio.outpp(0);
    slp_gpio.outpp(0);

    timer1.init(chopper_freq, TimerUtils::Mode::CenterAlignedUpTrig);

    auto & pwm_u = timer1.oc(1); 
    auto & pwm_v = timer1.oc(2); 
    auto & pwm_w = timer1.oc(3); 

    timer1.oc(4).init(TimerUtils::OcMode::UpValid, false);
    timer1.oc(4).setOutputState(true);

    timer1.oc(4).cvr() = timer1.arr()-1;
    timer1.oc(4).setIdleState(false);

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


    MP6540 mp6540{
        {pwm_u, pwm_v, pwm_w},
        {adc1.inj(1), adc1.inj(2), adc1.inj(3)}
    };

    mp6540.init();
    mp6540.setSoRes(1_K);
    
    SVPWM3 svpwm {mp6540};
    

    auto & u_ch = mp6540.ch(1);
    auto & v_ch = mp6540.ch(2);
    auto & w_ch = mp6540.ch(3);
    
    using AdcChannelEnum = AdcUtils::Channel;
    using AdcCycleEnum = AdcUtils::SampleCycles;

    adc1.init(
        {
            AdcChannelConfig{AdcChannelEnum::VREF, AdcCycleEnum::T28_5}
        },{
            AdcChannelConfig{AdcChannelEnum::CH1, AdcCycleEnum::T7_5},
            AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T7_5},
            AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T7_5},
            // AdcChannelConfig{AdcChannelEnum::CH1, AdcCycleEnum::T7_5},
            // AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T28_5},
            // AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T28_5},
            // AdcChannelConfig{AdcChannelEnum::CH1, AdcCycleEnum::T41_5},
            // AdcChannelConfig{AdcChannelEnum::CH4, AdcCycleEnum::T41_5},
            // AdcChannelConfig{AdcChannelEnum::CH5, AdcCycleEnum::T41_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enableAutoInject(false);


    real_t rad = 0;
    real_t open_rad = 0;

    Current3 uvw_curr = {0,0,0};
    Current3 uvw_curr_drift = {0,0,0};

    Current2 ab_curr = {0,0};
    Current2 dq_curr = {0,0};

    // Voltage bus_volt = 0;
    real_t est_rad;

    // real_t s_lpf_u_curr = 0;


    #define LPF(x,y) x = (((x >> 5) * 31 + (y >> 5)));
    // #define SLPF(x,y) x = (((x >> 2) * ((1 << 16) - 1) + (y >> 2)) >> 14);
    #define SLPF(x,y) x = ((x * ((1 << 14) - 1) + y) >> 14);

    auto update_curr = [&](){
        // #define LPF(x,y) x = (((x >> 4) * 15 + (y >> 4)));


        static Current3 uvw_curr_raw;
                
        LPF(uvw_curr_raw[0], real_t(u_ch));
        LPF(uvw_curr_raw[1], real_t(v_ch));
        LPF(uvw_curr_raw[2], real_t(w_ch));

        // SLPF(s_lpf_u_curr, uvw_curr_raw[0]);
        
        for(size_t i = 0; i < 3; i++){
            uvw_curr[i] = uvw_curr_raw[i] - uvw_curr_drift[i];
        }

        ab_curr = uvw_to_ab(uvw_curr);

        // bus_volt = data_to_volt(adc_data[3]);
    };


    auto cb = [&](){

        update_curr();

        odo.update();
        auto pos = ma730.getLapPosition();
        rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.7)  - frac(pos * 7) * real_t(TAU);
        // real_t open_pos = t * real_t(0.3);
        real_t open_pos = t * real_t(2.3);
        open_rad = frac(open_pos * 7) * real_t(TAU);
        // est_rad = atan2(ab_curr[1], ab_curr[0]) + real_t(7.2) + real_t(PI/2);
        est_rad = atan2(ab_curr[1], ab_curr[0]) + real_t(7.2) - real_t(PI);
        // setDQDuty(0, real_t(0.01), rad);
        // setDQDuty(0, real_t(0.01), open_rad);
        svpwm.setDuty(real_t(-0.3), rad);
        // svpwm.setDQDuty(Vector2(0,real_t(0.2)), open_rad);
        
        // setDQDuty(0, real_t(0.01), est_rad);
        // auto temp_dq_curr = ab_to_dq(ab_curr, rad);
        dq_curr = ab_to_dq(ab_curr, rad);
    };

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    for(size_t i = 0; i < 4000; ++i){
        update_curr();
    }
    
    u_ch.setBasis(uvw_curr[0]);
    v_ch.setBasis(uvw_curr[1]);
    w_ch.setBasis(uvw_curr[2]);

    // DEBUG_PRINTLN(real_t(u_ch), real_t(v_ch), real_t(w_ch));

    adc1.bindCb(AdcUtils::IT::JEOC, cb);
    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

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
        // (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3, (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1], est_rad, rad);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100), int(uvw_curr[1]*100), int(uvw_curr[2]*100));
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2]);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1]);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), real_t(adc1.inj(2)), real_t(adc1.inj(3)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_ch), real_t(v_ch), real_t(w_ch));
        DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), uint16_t(adc1.inj(1)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_ch), s_lpf_u_curr);
        // auto [a,b] = Vector2{real_t(0), real_t(0.2)}.rotated(open_rad);
        // DEBUG_PRINTLN(a,b);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, TIM1->CH1CVR, TIM1->CH4CVR, ADC1->IDATAR1);
        // TIM1->CH4CVR = 1000;
        // cb();
        // delay(10);
        // DEBUG_PRINTLN(spi1.cs_port.isIndexValid(0), spi1.cs_port.isIndexValid(1), spi1.cs_port.isIndexValid(2))
        // DEBUG_PRINTLN("0");
        // bmi.check();
        // delay(20);
    }
}