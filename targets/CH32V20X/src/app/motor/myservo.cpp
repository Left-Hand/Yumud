#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/utils/streamed_string_splitter.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "src/testbench/tb.h"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "drivers/Encoder/Encoder.hpp"
#include "drivers/Encoder/AnalogEncoder.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"

using namespace ymd;
using namespace ymd::hal;
namespace ymd::motorctl{
class ServoElectrics{
public:
    struct State{
        q24 duty;

        void reset(){
            duty = 0;
        }
    };

    struct Params{
        q24 curr_meas_raw;
        q24 curr_targ;
    };

    struct SenseConfig{
        q16 base;
        q16 gain;
    };

    struct Config{
        // SenseConfig sense_config;
        uint fc;
        q8 kp;
        uint fs;
    };

    ServoElectrics(const Config & cfg){
        reconf(cfg);
    }

    void reconf(const Config & cfg){
        curr_lpf_.reconf({cfg.fc, cfg.fs});
        kp_ = cfg.kp;
    }

    void reset(){
        curr_lpf_.reset();
        state_.reset();
    }
    void update(const Params params){
        curr_lpf_.update(params.curr_meas_raw);
        const auto curr_now = curr_lpf_.get();
        state_ = forward_state(state_, params.curr_targ, curr_now);
    }

    const State & get() const {return state_;}
private:
    using Lpf = dsp::LowpassFilter<q24>;
    // SenseConfig sense_cfg_;
    Lpf curr_lpf_;
    q8 kp_;


    State state_ = {};

    constexpr State forward_state(const State & state, const q24 curr_targ, const q24 curr_now) const {
        return State{
            .duty = state.duty + (curr_now - curr_targ) * kp_
        };
    }
};


class ServoDynamics{
public:
    struct State{
        q24 torque;
    };

    struct Params{
        q24 pos_meas;
        q24 spd_meas;
        q24 pos_ref;
        q24 spd_ref;
    };

    struct Config{
        q24 kp;
        q24 kd;
        uint fs;
    };

    ServoDynamics(const Config & cfg){
        reconf(cfg);
    }

    void reconf(const Config & cfg){
        kp_ = cfg.kp;
        kd_ = cfg.kd;
    }

    void update(const Params & params){
        state_ = {
            .torque = kp_ * (params.pos_meas - params.pos_ref)
                + kd_ * (params.spd_meas - params.spd_ref)
        };
    }

    const State & get() const {return state_;}
private:
    q24 kp_;
    q24 kd_;

    State state_;
};


class ServoCtrlSystem{
public:
    void update(){
        // encoder_.update().unwrap();
        // const auto lap_pos_raw = encoder_.read_lap_angle().unwrap();
        // fb_pos_td_.update(lap_pos_raw);

        // const auto pos_meas = fb_pos_td_.get()[0];
        // const auto spd_meas = fb_pos_td_.get()[1];

        // const auto pos_ref = cmd_pos_td_.get()[0];
        // const auto spd_ref = cmd_pos_td_.get()[1];

        // dynamics_.update({
        //     .pos_meas = pos_meas,
        //     .spd_meas = spd_meas,
        //     .pos_ref = pos_ref,
        //     .spd_ref = spd_ref
        // });

        // const auto torque = dynamics_.get().torque;

        // electric_.update({
        //     .curr_meas_raw = 0,
        //     .curr_targ = torque,
        // });

        // const auto duty = electric_.get().duty;
        // pwm_.set_dutycycle(duty);
    }

    ServoCtrlSystem(hal::AnalogInIntf & ana, hal::PwmIntf & pwm):
        encoder_(drivers::AnalogEncoder::Config{
            .volt_range = {0,0}, 
            .pos_range = {0,0}
        }, ana),
        pwm_(pwm){;}
private:    
    drivers::AnalogEncoder encoder_;
    using Td = dsp::TrackingDifferentiatorByOrders<2>;
    // Td fb_pos_td_;
    // Td cmd_pos_td_;

    // ServoDynamics dynamics_;
    // ServoElectrics electric_;
    hal::PwmIntf & pwm_;
};

}

class Joystick{
public: 
    virtual void update();
};

class AnalogJoystick{

};

// template<typename Q>
// static constexpr iq_t<Q> stepify(const iq_t<Q> x, const auto step){
//     return int(x / step) * step;
// }


#define UART hal::uart2
static constexpr uint32_t TIM_FREQ = 5000;
static constexpr uint32_t ISR_FREQ = TIM_FREQ / 2;
void myservo_main(){
    UART.init({576000});
    // UART.enable_single_line_mode(false);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();
    DEBUG_PRINTLN("powerup");


    // auto & led = portD[0];
    auto & led = portB[8];
    led.outpp(HIGH);

    // {
    //     auto & ledr = portC[13];
    //     ledr.outpp(HIGH);
    //     while(true){
    //         ledr.toggle();
    //         clock::delay(200ms);
    //         DEBUG_PRINTLN(clock::millis().count());
    //     }
    // }

    while(true){

        hal::timer3.init({TIM_FREQ, TimerCountMode::CenterAlignedUpTrig});

        auto & pwm_pos = hal::timer3.oc<1>();
        auto & pwm_neg = hal::timer3.oc<1>();

        auto set_dutycycle = [&](real_t duty){
            duty = CLAMP2(duty, 0.8_r);
            const auto duty_is_forward = duty > 0.0_r;
            if(duty_is_forward){
                pwm_pos.set_dutycycle(duty);
                pwm_neg.set_dutycycle(0.0_r);
            }else{
                pwm_pos.set_dutycycle(0.0_r);
                pwm_neg.set_dutycycle(-duty);
            }
        };
        pwm_pos.init({});
        pwm_neg.init({});

        while(true){
            const auto t = clock::time();
            const auto duty = sin(t) * 0.5_r;
            set_dutycycle(duty);
            clock::delay(1ms);
        }
    }

    auto & can = can1;

    auto & mode1_gpio   = portB[1];
    auto & phase_gpio   = portA[7];
    // auto & en_gpio      = portA[6];
    // auto & mode2_gpio   = portA[5];
    phase_gpio.outpp();

    using ButterLpf = dsp::ButterLowpassFilter<iq_t<16>, 4>;
    using ButterLpfConfig = typename ButterLpf::Config;

    using RcLpf = dsp::LowpassFilter<iq_t<20>>;
    using RcLpfConfig = typename RcLpf::Config;

    ButterLpf curr_filter = {ButterLpfConfig{
        .fc = 40,
        .fs = ISR_FREQ,
    }};

    RcLpf spin_filter = {RcLpfConfig{
        .fc = 2,
        .fs = ISR_FREQ,
    }};

    auto init_adc = []{
        adc1.init(
            {
                {AdcChannelNth::VREF, AdcSampleCycles::T28_5}
            },{
                {AdcChannelNth::CH4, AdcSampleCycles::T7_5},
                {AdcChannelNth::CH1, AdcSampleCycles::T28_5},
            },
            {}
        );

        // adc1.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
        adc1.set_injected_trigger(AdcInjectedTrigger::T3CC4);
        // adc1.enableContinous();
        adc1.enable_auto_inject(DISEN);
    };

    // can.init(CanBaudrate::_1M, CanMode::Internal);
    can.init({CanBaudrate::_1M});
    init_adc();

    auto & ain1 = adc1.inj<1>();
    auto & ain2 = adc1.inj<2>();


    hal::timer3.init({TIM_FREQ, TimerCountMode::CenterAlignedUpTrig});

    real_t sense_raw_volt;
    auto & pwm = hal::timer3.oc<1>();
    auto & pwm_trig = hal::timer3.oc<4>();
    pwm.init({});
    pwm_trig.init({.install_en = DISEN});
    pwm_trig.set_dutycycle(0.001_r);
    hal::timer3.set_trgo_source(TimerTrgoSource::OC4R);

    // real_t curr_cmd = 0;

    bool duty_is_forward = false;
    auto set_dutycycle = [&](real_t duty){
        duty = CLAMP2(duty, 0.8_r);
        duty_is_forward = duty > 0.0_r;
        phase_gpio = BoolLevel::from(duty_is_forward);
        pwm.set_dutycycle(ABS(duty));
    };

    hal::adc1.attach(AdcIT::JEOC, {0,0}, [&]{
        sense_raw_volt = ain1.get_voltage();
        curr_filter.update(sense_raw_volt);
        spin_filter.update(ain2.get_voltage());
    });


    led.outpp();
    while(true){
        led = BoolLevel::from((clock::millis() % 400).count() > 200);
            // auto pwm = hal::
        mode1_gpio.outpp(HIGH);
        // slp_gpio
        // const auto duty = 0.6_r * sinpu(clock::time());


        // curr_cmd = ABS(0.035_r * sin(clock::time() * 4))+ 0.035_r;

        // const auto duty_cmd = 0.4_r * sin(clock::time());
        // const auto pos_cmd = int(sin(clock::time()/3) * 5) * 0.2_r;
        const auto pos_cmd = 0.81_r * sin(clock::time() * 3);
        const auto duty_cmd = 0.4_r * dsp::adrc::ssqrt(1.65_r + pos_cmd - spin_filter.get());
        // const auto duty_cmd = 0.4_r * sign(sinpu(clock::time() * 10));
        set_dutycycle(duty_cmd);
        // curr_cmd = ain2.get_voltage() / 3.3_r * 0.015_r + 0.015_r;
        // phase_gpio = BoolLevel::from((clock::millis() % 400).count() > 200);
        // pwm.set_dutycycle(ABS(duty));
        // duty = CLAMP(ain2.get_voltage() / 3.3_r, 0, 0.9_r);
        // pwm.set_dutycycle(duty);

        // led = HIGH;
        // clock::delay(200ms);
        // led = LOW;
        // clock::delay(200ms);
        // DEBUG_PRINTLN(
        //     real_t(pwm) 
        //     ,bool(portA[6].read()) 
        //     ,bool(phase_gpio.read())
        //     ,real_t(ain1)
        //     ,real_t(ain2)
        // );
        // DEBUG_PRINTLN(clock::millis().count(), UART.available());
        // DEBUG_PRINTLN(UART.available());
        // while(UART.available()){
        //     char chr;
        //     UART.read1(chr);
        //     DEBUG_PRINTLN(chr);
        // }
        // DEBUG_PRINTLN(msg);
        // DEBUG_PRINTLN("before", can.pending());
        // clock::delay(200ms);
        // DEBUG_PRINTLN("after", can.pending());
        // clock::delay(2ms);

        // constexpr auto msg = CanMsg::from_remote(CanStdId(0xff));
        // can.write(msg);
        const auto curr = duty_is_forward ? curr_filter.get() : -curr_filter.get();
        DEBUG_PRINTLN_IDLE(
            duty_cmd,
            sense_raw_volt, 
            curr, 
            spin_filter.get(),
            pos_cmd
        );
        // for(auto i = 0; i < 1000; ++i){
        //     DEBUG_PRINTLN(portB[9].read());
        // }
    }

}
