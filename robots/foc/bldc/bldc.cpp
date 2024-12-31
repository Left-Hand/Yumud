#include "../sys/core/system.hpp"

#include "../types/vector3/vector3_t.hpp"
#include "../types/quat/Quat_t.hpp"


#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "drivers/Actuator/Bridge/MP6540/mp6540.hpp"
#include "drivers/Actuator/SVPWM/svpwm.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"

#include <ostream>

using namespace ymd;
using namespace ymd::drivers;



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

static __inline real_t sign_sqrt(const real_t x){
    return x < 0 ? -sqrt(-x) : sqrt(x);
};

static __inline real_t smooth(const real_t x){
    return x - sin(x);
}

static __inline real_t f(const real_t x){
    // return sin(7 * x) / 7 + sin(5 * x) / 5 + sin(3 * x)/ 3 + sin(x);
    // return sin(5 * x) / 5 + sin(3 * x)/ 3 + sin(x);
    return sin(x);
}

real_t pos;
int bldc_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    auto & en_gpio = portA[11];
    auto & slp_gpio = portA[12];

    en_gpio.outpp(0);
    slp_gpio.outpp(0);

    timer1.init(chopper_freq, TimerUtils::Mode::CenterAlignedUpTrig);

    auto & pwm_u = timer1.oc(1); 
    auto & pwm_v = timer1.oc(2); 
    auto & pwm_w = timer1.oc(3); 

    timer1.oc(4).init(TimerUtils::OcMode::UpValid, false)
                .setOutputState(true)
                .setIdleState(false);
    
    timer1.oc(4).cvr() = timer1.arr() - 1;

    pwm_u.init();
    pwm_v.init();
    pwm_w.init();

    spi1.init(18_MHz);
    spi1.bindCsPin(portA[15], 2);
    spi1.bindCsPin(portA[0], 0);


    can1.init(1_MHz);

    BMI160 bmi{spi1, 0};
    bmi.init();

    MA730 ma730{spi1, 2};
    ma730.init();

    // for(size_t i = 0; i < 1000; ++i) {
    //     bmi.update();
    //     // auto [x,y,z] = bmi.getAcc();
    //     auto [x,y,z] = bmi.getGyr();
    //     DEBUG_PRINTLN(x,y,z);
    //     delay(2);
    //     // DEBUGGER << std::endl;
    // }
    Odometer odo{ma730};
    odo.init();


    MP6540 mp6540{
        {pwm_u, pwm_v, pwm_w},
        {adc1.inj(1), adc1.inj(2), adc1.inj(3)}
    };

    mp6540.init();
    mp6540.setSoRes(1_K);
    
    SVPWM3 svpwm {mp6540};
    

    auto & u_sense = mp6540.ch(1);
    auto & v_sense = mp6540.ch(2);
    auto & w_sense = mp6540.ch(3);
    
    using AdcChannelEnum = AdcUtils::ChannelIndex;
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

    Current3 uvw_curr = {0,0,0};
    Current3 uvw_curr_bias = {14.62_r,14.68_r,14.68_r};

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
                
        LPF(uvw_curr_raw[0], real_t(u_sense));
        LPF(uvw_curr_raw[1], real_t(v_sense));
        LPF(uvw_curr_raw[2], real_t(w_sense));

        // SLPF(s_lpf_u_curr, uvw_curr_raw[0]);
        
        for(size_t i = 0; i < 3; i++){
            uvw_curr[i] = uvw_curr_raw[i];
        }

        ab_curr = uvw_to_ab(uvw_curr);
        dq_curr = ab_to_dq(ab_curr, rad);
        // bus_volt = data_to_volt(adc_data[3]);
    };


    uint32_t dt;
    auto cb = [&](){
        auto m = micros();
        update_curr();

        odo.update();
        auto pos = ma730.getLapPosition();
        rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.7)  - frac(pos * 7) * real_t(TAU);
        // real_t open_rad = t * real_t(12.3);
        // real_t targ_pos = t * real_t(1);
        // real_t targ_pos = t * real_t(10);
        // real_t targ_pos = t * real_t(10);
        // real_t targ_pos = 200;
        // real_t targ_pos = smooth(t) * 20;
        // real_t targ_pos = 4 * floor(2 * t);
        real_t targ_pos = 10 * f(t);
        // real_t targ_pos = sin(4 * t) * real_t(0.2);
        // real_t targ_pos = sin(t) * real_t(12.3);
        // real_t targ_pos = 20 * sin(t);
        // real_t targ_pos = 0;
        // open_rad = frac(open_pos * 7) * real_t(TAU);
        // est_rad = atan2(ab_curr[1], ab_curr[0]) + real_t(7.2) + real_t(PI/2);
        // est_rad = atan2(ab_curr[1], ab_curr[0]) + real_t(7.2) - real_t(PI);
        // setDQDuty(0, real_t(0.01), rad);
        // setDQDuty(0, real_t(0.01), open_rad);

        // svpwm.setDuty(real_t(0.3) * sin(t), rad);
        // signs
        // svpwm.setDuty(0.1_r, frac(targ_pos) * real_t(7*TAU));
        // svpwm.setDuty(CLAMP2((-0.17_r) * sign_sqrt(targ_pos - odo.getPosition()), 0.4_r) , rad);
        svpwm.setDuty(CLAMP2(- 0.2_r * (targ_pos - odo.getPosition()), 0.5_r) , rad);
        
        // setDQDuty(0, real_t(0.01), est_rad);
        // auto temp_dq_curr = ab_to_dq(ab_curr, rad);
        dt = micros() - m;
    };

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    for(size_t i = 0; i < 400; ++i){
        update_curr();
        delay(1);
        // DEBUG_PRINTLN(real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // DEBUG_PRINTLN(real_t(adc1.inj(1)), real_t(adc1.inj(2)), real_t(adc1.inj(3)));
        // delay(2);
    }
    
    u_sense.setBasis(uvw_curr_bias[0]);
    v_sense.setBasis(uvw_curr_bias[1]);
    w_sense.setBasis(uvw_curr_bias[2]);

    adc1.bindCb(AdcUtils::IT::JEOC, cb);
    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

    en_gpio = true;
    slp_gpio = true;


    while(true){
        // auto pos = ma730.getLapPosition();

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;
        pos = odo.getPosition();
        // auto _t = real_t(0);

        // DEBUG_PRINTLN(odo.getPosition());
        // DEBUG_PRINTLN((odo.getPosition()), real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2], real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2]);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), ab_curr[0],ab_curr[1]);
        // delay(2);
        // DEBUG_PRINTLN(pos, dq_curr[0],dq_curr[1], dt);
        if(DEBUGGER.pending() == 0)DEBUG_PRINTLN(pos, dq_curr[0],dq_curr[1], dt);

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
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // pwm_u = sin(t) * 0.5_r + 0.5_r;
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // delay(5);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), uint16_t(adc1.inj(1)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), s_lpf_u_curr);
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