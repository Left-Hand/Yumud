#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "drivers/Actuator/Bridge/MP6540/mp6540.hpp"
#include "drivers/Actuator/SVPWM/svpwm.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"
#include "drivers/Actuator/Bridge/DRV8301/DRV8301.hpp"

#include "smo/SmoObserver.hpp"
#include "lbg/RolbgObserver.hpp"
#include "nonlinear/NonlinearObserver.hpp"
#include "utils.hpp"

#include <ostream>
#include "sys/core/system.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;


using Sys::t;



static __inline real_t f(const real_t x){
    // return sin(7 * x) / 7 + sin(5 * x) / 5 + sin(3 * x)/ 3 + sin(x);
    // return sin(5 * x) / 5 + sin(3 * x)/ 3 + sin(x);
    return sin(x);
}

// template<size_t N>
// class SimpleLPF{
// protected:
//     iq_t output;
// public:
//     iq_t update(const iq_t x){
//         // return ((x * 31 + y) >> 5);
//     }
// }

__fast_inline iq_t LPF5(const iq_t x, const iq_t y){
    // return (x * 31 + y) >> 5;
    // return y;
    auto temp = x * 31 + y;
    if((int32_t(temp.value) & 31) > 16){
        return (temp >> 5) + iq_t(_iq(1));
    }else{
        return (temp) >> 5;
    }
}

__fast_inline iq_t LPF6(const iq_t x, const iq_t y){
    return (x * 63 + y) >> 6;
}

template<size_t N>
__fast_inline iq_t LPFN(const iq_t x, const iq_t y){
    return (x * ((1 << N) - 1) + y) >> N;
}

__fast_inline iq_t LPF7(const iq_t x, const iq_t y){
    return (x * 127 + y) >> 7;
}

__fast_inline iq_t LPF4(const iq_t x, const iq_t y){
    return (x * 15 + y) >> 4;
}

__fast_inline iq_t LPF3(const iq_t x, const iq_t y){
    return (x * 7 + y) >> 3;
}

// template<typename T, size_t N>
// class AverageFilter{
// protected:
//     std::array<T, N> data_;
//     size_t index = 0;
//     bool inited = false;
// public:
//     void reset(const T x = 0){
//         data_.fill(0);
//     }

//     T update(const T x){
//         if(!inited){
//             inited = true;
//             data_.fill(x);
//             return x;
//         }

//         data_[index] = x;
//         index = (index + 1) % N;

//         T sum = 0;
//         for(const auto & x : data_){
//             sum += x;
//         }

//         return sum / int(N);
//     }

// };


template<typename T, size_t N>
class AverageFilter {
protected:
    scexpr T invN = T(1.0 / N);

    std::array<T, N> data_;
    size_t index = 0;
    bool inited = false;
    T sum = 0;
    

public:
    void reset(const T x) {
        data_.fill(x);
        sum = x * N;
        inited = true;
    }

    T update(const T x) {
        if (unlikely(!inited)) {
            inited = true;
            reset(x);
            return x;
        }

        T old_value = data_[index];
        data_[index] = x;
        index = (index + 1) % N;

        sum += x - old_value;

        return sum * invN;
    }
};

__fast_inline iq_t LPF(const iq_t x, const iq_t y){
    // const iq_t temp = x * 31 + y;

    // if(int32_t(temp.value) & 16){
    //     return ((temp >> 5) + (iq_t(1) >> GLOBAL_Q));
    // }else{
    //     return (temp >> 5);
    // }
    return y;
    // return (() >> 5);
}



class CurrentSensor{
protected:
    AnalogInChannel & _u_sense;
    AnalogInChannel & _v_sense;
    AnalogInChannel & _w_sense;
    UvwCurrent _uvw_curr;
    AbCurrent _ab_curr;
    DqCurrent _dq_curr;
public:
    CurrentSensor(AnalogInChannel & u_sense, AnalogInChannel & v_sense, AnalogInChannel & w_sense): 
        _u_sense(u_sense), _v_sense(v_sense), _w_sense(w_sense){}

    void updatUVW(){

        // #define LPF(x,y) x = ((x * 31 + y) >> 5);
        // #define LPF(x,y) x = (((x >> 5) * 31 + (y >> 5)));
        // #define LPF(x,y) x = (((x >> 4) * 15 + (y >> 4)));
        // #define LPF(x,y) x = (((x >> 3) * 7 + (y >> 3)));
        // #define LPF(x,y) x = (((x >> 1)  + (y >> 1)));
        // #define LPF(x,y) x = y;

        _uvw_curr[0] = LPF5(_uvw_curr[0], real_t(_u_sense));
        _uvw_curr[1] = LPF5(_uvw_curr[1], real_t(_v_sense));
        _uvw_curr[2] = LPF5(_uvw_curr[2], real_t(_w_sense));
    }

    void updateAB(){
        _ab_curr = uvw_to_ab(_uvw_curr);
    }

    void updateDQ(const real_t rad){
        const auto dq_curr = ab_to_dq(_ab_curr, rad);
        _dq_curr[0] = LPF5(_dq_curr[0], real_t(dq_curr[0]));
        _dq_curr[1] = LPF5(_dq_curr[1], real_t(dq_curr[1]));
    }

    void update(const real_t rad){
        updatUVW();
        updateAB();
        updateDQ(rad);
    }

    auto & uvw()const{return _uvw_curr;}
    auto & uvw(){return _uvw_curr;}
    auto & ab()const{return _ab_curr;}
    auto & ab(){return _ab_curr;}
    auto & dq()const{return _dq_curr;}
    auto & dq(){return _dq_curr;}
};


class SensorlessEncoder:public EncoderIntf{
protected:
    SensorlessObserverIntf & ob_;
public:
    SensorlessEncoder(
        SensorlessObserverIntf & ob
    ):
        ob_(ob){;}
    real_t getLapPosition() = 0;
    void update() = 0;
    void init() = 0;
    bool stable() = 0;
};


int bldc_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    delay(200);
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

    // BMI160 bmi{spi1, 0};

    // bmi.init();

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
    
    using AdcChannelIndex = AdcUtils::ChannelIndex;
    using AdcCycles = AdcUtils::SampleCycles;

    adc1.init(
        {
            {AdcChannelIndex::VREF, AdcCycles::T28_5}
        },{
            {AdcChannelIndex::CH1, AdcCycles::T7_5},
            {AdcChannelIndex::CH4, AdcCycles::T7_5},
            {AdcChannelIndex::CH5, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T41_5},
        }
    );

    // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T1CC4);
    // adc1.enableContinous();
    adc1.enableAutoInject(false);

    real_t pos = 0;
    real_t targ_pos;
    real_t rad = 0;

    // UvwCurrent uvw_curr = {0,0,0};
    UvwCurrent uvw_curr_bias = {14.62_r,14.68_r,14.68_r};
    real_t est_rad;

    CurrentSensor current_sensor = {u_sense, v_sense, w_sense};
    // CurrentSensor current_sensor = {adc1.inj(1), adc1.inj(2), adc1.inj(3)};

    // uint32_t dt;

    std::array<real_t, 2> ab_volt;

    // scexpr real_t r_ohms = 7.1_r;
    // scepxr real_t l_mh = 1.45_r;
    SmoObserver smo_ob = {0.7_r, 0.04_r, 8.22_r, 0.3_r};
    RolbgObserver lbg_ob;
    NonlinearObserver::Config nlr_conf = {
        .phase_inductance = 1.45E-3_r,
        .phase_resistance = 7.1_r,
        .observer_gain = 0.1_r,
        // .observer_gain = 0.3_r,
        // .pm_flux_linkage = 1.58e-3_r,
        .pm_flux_linkage = 3.58e-4_r,
        .freq = chopper_freq/2,
    };

    NonlinearObserver nlr_ob = {
        nlr_conf
    };

    // auto m = micros();
    auto cb = [&]{

        // odo.update();

        // targ_pos = real_t(6.0) * sin(2 * t);
        targ_pos = real_t(1.0) * t;
        pos = odo.getPosition();
        // auto pos = odo.getLapPosition();
        current_sensor.updatUVW();
        current_sensor.updateAB();

        // rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.5)  - frac(pos * 7) * real_t(TAU);
        // rad = real_t(TAU + PI / 2) + real_t(PI / 2) + real_t(0.5)  - frac(targ_pos * 7) * real_t(TAU);
        // if(t < 2){
        //     rad = -frac(targ_pos * 7) * real_t(TAU);
        // }else{
        
        // rad = -frac(targ_pos * 7) * real_t(TAU);
        // rad =  + real_t(PI/2);
        // rad = smo_ob.getTheta() + real_t(-PI);
        // fmod(t,8)
        // rad = smo_ob.getTheta() - real_t(PI/2) + real_t(PI/2 + 1.4);


        auto change = real_t(1.5) * real_t(PI) * sin(4 * t);
        if(change > real_t(PI/2) || change < -real_t(PI/2)){
            // rad = smo_ob.theta() - real_t(PI/2) + CLAMP2(change, real_t(PI));
            // rad = lbg_ob.theta() - real_t(PI/2) + CLAMP2(change, real_t(PI));
            // rad = nlr_ob.theta() - real_t(PI/2) + CLAMP2(change, real_t(PI * 0.9));
        }
        // rad = lbg_ob.theta();
        // rad = smo_ob.theta();
        rad = nlr_ob.theta() + 0.8_r;
        // rad = nlr_ob.theta();
        
        // rad = frac(4*t)*real_t(TAU);
        // rad = lbg_ob.theta();


        // rad = smo_ob.getTheta() + real_t(PI/2);
        // rad = smo_ob.getTheta() + real_t(PI/2) - real_t(PI/2);
        // rad = smo_ob.getTheta() + real_t(-PI/2) + real_t(PI/2 ) * sin(4 * t);
        // rad = smo_ob.getTheta() + real_t(-PI/2) + real_t(PI/2-0.3);
        // rad = smo_ob.getTheta() + real_t(-PI/2) + real_t(-1.9);
        // rad = smo_ob.getTheta() + real_t(-PI/2) + real_t(3.1);
        // rad = real_t(TAU + PI / 2) + real_t(PI / 2)  - frac(targ_pos * 7) * real_t(TAU);
        // rad = -real_t(PI / 2)  + real_t(PI) - frac(pos * 7) * real_t(TAU);
        // rad = smo_ob.getTheta() + real_t(-PI/2) - real_t(PI/2 + 0.3) * 1;
        // }
        // auto err = targ_pos - pos;
        // real_t open_rad = t * real_t(12.3);
        // real_t targ_pos = t * real_t(1);
        // real_t targ_pos = t * real_t(10);
        // real_t targ_pos = t * real_t(10);
        // real_t targ_pos = 200;
        // real_t targ_pos = smooth(t) * 20;
        // real_t targ_pos = 4 * floor(2 * t);
        // real_t targ_pos = 40 * f(t);
        // real_t targ_pos = real_t(50.0 / 7) * sin(t);
        // real_t targ_pos = real_t(50.0/7) * sin(t * 3);
        // real_t targ_pos = real_t(50.0/7) * t;

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
        // auto curr = CLAMP2(- 0.2_r * (targ_pos - odo.getPosition()), 0.5_r);
        // signs
        // svpwm.setDuty(0.1_r, frac(targ_pos) * real_t(7*TAU));
        // svpwm.setDuty(CLAMP2((-0.17_r) * sign_sqrt(targ_pos - odo.getPosition()), 0.4_r) , rad);
        // svpwm.setDuty(curr, rad - curr);
        // svpwm.setDuty(curr, rad + curr);
        // svpwm.setDuty(0.1_r, rad);
        // svpwm.setDQVolt(0, 2.0_r, rad);
        // real_t v = -3 * sin(t);
        // real_t v = 3;
        // real_t v = 4.0_r;
        // real_t v = 4.0_r;
        real_t v = 6.0_r;
        ab_volt = {v * cos(rad), v * sin(rad)};
        svpwm.setABVolt(ab_volt[0], ab_volt[1]);
        const auto & ab_curr = current_sensor.ab();

        // smo_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);

        current_sensor.updateDQ(smo_ob.theta());
        // svpwm.setVolt(2, rad);
        // dt = micros() - m;
        // m = micros();
        // dt = micros() ;
        // setDQDuty(0, real_t(0.01), est_rad);
        // auto temp_dq_curr = ab_to_dq(ab_curr, rad);

    };

    real_t hfi_result;

    [[maybe_unused]] auto cb_hfi = [&]{

        static int cnt = 0;
        // scexpr int hfi_freq = 4096;
        // scexpr int hfi_freq = 2500;
        // scexpr int hfi_freq = 1024;
        // scexpr int hfi_freq = 512;
        // scexpr int hfi_freq = 256;
        // scexpr int divider = chopper_freq / 2 / hfi_freq;
        scexpr int divider = 8;
        cnt = (cnt + 1) % divider;




        real_t hfi_base_volt = 3.2_r;
        real_t hfi_rad = real_t(TAU) * real_t(cnt) / divider;
        real_t hfi_c = cos(hfi_rad);
        real_t hfi_out = hfi_base_volt * hfi_c;

        real_t openloop_base_volt = 6.0_r;
        real_t openloop_rad = -frac(13.1_r * t)*real_t(TAU);
        real_t openloop_c = cos(openloop_rad);
        real_t openloop_s = sin(openloop_rad);
        // real_t s = sin(hfi_rad);

        ab_volt = {hfi_out + openloop_base_volt * openloop_c, openloop_base_volt * openloop_s};
        svpwm.setABVolt(ab_volt[0], ab_volt[1]);

        current_sensor.updatUVW();
        current_sensor.updateAB();
        current_sensor.updateDQ(0);

        // real_t mul = current_sensor.ab()[1] * s;
        real_t mul = current_sensor.ab()[1] * hfi_c;
        // real_t last_hfi_result = hfi_result;
        // hfi_result = LPF(last_hfi_result, mul);
        static AverageFilter<iq_t, 64> hfi_filter;
        hfi_result = hfi_filter.update(mul);
        // hfi_result = LPFN<9>(hfi_result, mul);

        // hfi_result = mul;
        
        // dt = micros() - m;
    };

    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    for(size_t i = 0; i < 400; ++i){
        current_sensor.updatUVW();
        current_sensor.updateAB();
        delay(1);
    }
    
    u_sense.setBasis(uvw_curr_bias[0]);
    v_sense.setBasis(uvw_curr_bias[1]);
    w_sense.setBasis(uvw_curr_bias[2]);

    adc1.bindCb(AdcUtils::IT::JEOC, cb);
    // adc1.bindCb(AdcUtils::IT::JEOC, cb_hfi);
    adc1.enableIT(AdcUtils::IT::JEOC, {0,0});

    en_gpio = true;
    slp_gpio = true;


    while(true){
        // auto pos = ma730.getLapPosition();

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;

        // auto _t = real_t(0);

        // DEBUG_PRINTLN(odo.getPosition());
        // DEBUG_PRINTLN((odo.getPosition()), real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2], real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2]);
        // if(DEBUGGER.pending() == 0)DEBUG_PRINTLN((odo.getPosition()), ab_curr[0],ab_curr[1]);
        // delay(2);
        // DEBUG_PRINTLN(pos, dq_curr[0],dq_curr[1], dt);
        [[maybe_unused]] const auto & uvw_curr = current_sensor.uvw();
        [[maybe_unused]] const auto & dq_curr = current_sensor.dq();
        [[maybe_unused]] const auto & ab_curr = current_sensor.ab();


        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(pos, ab_curr[0], ab_curr[1], ab_volt[0], ab_volt[1], smo_ob.getTheta(),  dt > 100 ? 1000 + dt : dt);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(pos, ab_curr[0], ab_curr[1], lbg_ob._e_alpha, lbg_ob._e_beta,  dt > 100 ? 1000 + dt : dt);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(ab_curr[0], ab_curr[1], lbg_ob._e_alpha, lbg_ob.theta());
        if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(nlr_ob.theta(), smo_ob.theta(), lbg_ob.theta(), uvw_curr[0]);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(ab_curr[1], ab_volt[0], hfi_result, acos(hfi_result * real_t(1 / 0.045 )));
        
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(hfi_result);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(ab_volt[0]);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(ab_curr[0], ab_curr[1], ab_volt[0]);
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dq_curr[0], dq_curr[1], targ_pos, pos, smo_ob.getTheta(), dt > 100 ? 1000 + dt : dt);
        // delay(2);

        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(rad, sin(rad), cos(rad), atan2(sin(rad), cos(rad)));
        // if(DEBUGGER.pending() == 0) DEBUG_PRINTLN(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dt > 100 ? 1000 + dt : dt);
        Sys::Clock::reCalculateTime();

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