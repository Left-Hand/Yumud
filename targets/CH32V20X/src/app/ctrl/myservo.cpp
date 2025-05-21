#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/fp/matchit.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "drivers/Encoder/odometer.hpp"

#include "algo/interpolation/cubic.hpp"

#include "robots/rpc/rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "drivers/Encoder/Encoder.hpp"
#include "drivers/Encoder/AnalogEncoder.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"

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
    using Lpf = dsp::LowpassFilter_t<q24>;
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
        // const auto lap_pos_raw = encoder_.get_lap_position().unwrap();
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
        // pwm_.set_duty(duty);
    }

    ServoCtrlSystem(hal::AnalogInIntf & ana, hal::PwmIntf & pwm):
        encoder_(drivers::AnalogEncoder::Config{.volt_range = {}, .pos_range = {}}, ana),
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
#define UART hal::uart2

void myservo_main(){
    UART.init(576000);
    // UART.enable_single_line_mode(false);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync();

    // auto & led = portD[0];
    auto & led = portB[8];

    auto & mode1_gpio   = portB[1];
    auto & phase_gpio   = portA[7];
    // auto & en_gpio      = portA[6];
    auto & mode2_gpio   = portA[5];

    phase_gpio.outpp();
    // en_gpio.outpp(HIGH);

    hal::timer3.init(1000);
    auto & pwm = hal::timer3.oc(1);
    pwm.init({});
    timer3.attach(TimerIT::Update, {0,0}, [&]{
        // const auto duty = sin(clock::time());
        // phase_gpio = BoolLevel::from(duty > 0);
        // pwm.set_duty(ABS(duty));

    });

    led.outpp();
    while(true){
        led = BoolLevel::from((clock::millis() % 400).count() > 200);
            // auto pwm = hal::
        mode1_gpio.outpp(HIGH);
        // slp_gpio
        const auto duty = sin(3 * clock::time());
        const auto level = duty > 0.0_r;
        phase_gpio = BoolLevel::from(level);
        // phase_gpio = BoolLevel::from((clock::millis() % 400).count() > 200);
        pwm.set_duty(ABS(duty));
        // led = HIGH;
        // clock::delay(200ms);
        // led = LOW;
        // clock::delay(200ms);
        DEBUG_PRINTLN(
            real_t(pwm), 
            bool(portA[6].read()), 
            bool(phase_gpio.read())
            , duty, level
        );
        // DEBUG_PRINTLN(clock::millis().count(), UART.available());
        // DEBUG_PRINTLN(UART.available());
        // while(UART.available()){
        //     char chr;
        //     UART.read1(chr);
        //     DEBUG_PRINTLN(chr);
        // }
    }

}
