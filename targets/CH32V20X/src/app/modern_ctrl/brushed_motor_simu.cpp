#include "src/testbench/tb.h"
#include "dsp/controller/adrc/linear/leso2o.hpp"

#include "middlewares/rpc/repl_server.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/uart/uartsw.hpp"

using namespace ymd;
using namespace ymd::robots;
using namespace ymd::dsp;

#if 0

namespace ymd::dsp{
//跟踪微分器 用于平滑输入

class CommandShaper1{
public:
    using Self = CommandShaper1;

    struct Config{
        iq12 kp;
        iq12 kd;
        iq16 max_spd;
        iq16 max_acc;
        uint32_t fs;
    };

    using E = iq16;
    using T = iq16;

    using State = std::array<iq20, 2>;

    
    CommandShaper1(const Config & cfg){
        reset();
        reconf(cfg);
    }

    //impure fn
    void update(const T targ){
        // __nopn(1);
        state_ = forward(*this, state_, targ);
    }

    //impure fn
    constexpr void reset(){
        // pass
    }

    //impure fn
    constexpr void reconf(const Config & cfg){
        kp_ = cfg.kp;
        kd_ = cfg.kd;
        dt_ = 1_iq16 / cfg.fs;
        max_spd_ = cfg.max_spd;
        max_acc_ = cfg.max_acc;
    }
    
    //impure fn
    [[nodiscard]]
    constexpr const State state() const {
        return {
            state_[0],
            state_[1] * 0.6_r
            // state_[1]
        };
    }
// private:
public:
    iq20 kp_;
    iq20 kd_;
    iq20 dt_;
    iq20 max_spd_;
    iq20 max_acc_;
    State state_ {};

    dsp::TrackingDifferentiatorByOrders<2> lpf = {dsp::TrackingDifferentiatorByOrders<2>::Config{
        .r = 80.0_r,
        .fs = 1000
    }};

    //pure fn
    [[nodiscard]]
    static constexpr __fast_inline State 
    // static State 
    forward(Self & self, const State & state, const T u0){
        // const auto r_3 = r_2 * r;
        const auto dt = self.dt_;
        // const auto max_spd = self.max_spd_;
        // const auto max_acc = self.max_acc_;

        const auto pos = state[0];
        const auto spd = state[1];
        // const auto acc = state[2];

        // const auto raw_a = ((iq12(self.kp_) * (<iq12>(u0 - pos)))
        //      - (self.kd_ * spd));
        // DEBUG_PRINTLN(raw_a, self.max_acc_);

        // static dsp::LowpassFilter<real_t> lpf = {dsp::LowpassFilter<real_t>::Config{
        //     .fc = 300,
        //     .fs = 1000
        // }};

        // static 

        self.lpf.update(u0);
        const auto u = self.lpf.state()[0];
        // const auto u = u0;
        const auto e1 = u - pos;
        const auto dist = ABS(e1);
        // const auto expect_spd = CLAMP2(SIGN_AS(std::sqrt(2.0_r * self.max_acc_ * dist), e1), self.max_spd_);
        // const auto expect_spd = fixed_t<16>(CLAMP2(self.lpf.state()[1] + SIGN_AS(std::sqrt(2.0_r * self.max_acc_ * dist), e1), self.max_spd_));
        // DEBUG_PRINTLN(u0, u, pos, spd, expect_spd, self.max_spd_, self.lpf.state());
        
        // DEBUG_PRINTLN(expect_spd);
        // if(dist > 0.1_r){
        if(true){
            auto expect_spd = std::copysign(std::sqrt(2.0_r * self.max_acc_ * dist), e1);
            // auto expect_spd = std::copysign(std::sqrt(1.57_r * self.max_acc_ * dist), e1);
            if(spd * self.lpf.state()[1] < 0) expect_spd += self.lpf.state()[1];
            const auto spd_cmd = iq20(CLAMP2(expect_spd, self.max_spd_));
            return {
                pos + spd * dt, 
                STEP_TO(spd, spd_cmd, iq20((self.max_acc_)* self.dt_)),
            };
        }else{
            return {
                pos + spd * dt, 
                // STEP_TO(spd, iq16(0), iq16()),
                CLAMP2(spd + CLAMP2( -self.kd_ * spd + self.kp_ * e1, self.max_acc_) * dt, self.max_spd_),
            };
        }

    }

    
};

}



void test_burshed_motor(){
    const auto tau = 80.0_r;
    dsp::TdVec2 td{{
        .kp = tau * tau,
        .kd = 2 * tau,
        .x2_limit = 60.0_r,
        .x3_limit = 1000.0_r,
        .fs = 1000
    }};



    [[maybe_unused]]
    auto test_td = [&](const auto t){
        // const auto u = 6 * Vec2<iq16>::RIGHT.rotated(real_t(TAU) * t);
        // const auto [x,y] = sincos(real_t(TAU) * t);
        // const auto m = sin(3 * real_t(TAU) * t);
        // const auto [x,y] = sincos(ret);
        // const auto u = Vec2<iq16>{CLAMP(70 * x, -30, 30), 6 * y};
        // const auto u = Vec2<iq16>{CLAMP(70 * x, -5, 5) + m, 0};
        // const auto u = Vec2<iq16>{10 * frac(t * 3), 0};
        const auto u = Vec2<iq16>{10 * frac(t), 0};
        
        // const auto u = Vec2<iq16>{CLAMP(70 * x, -30, 30), 0};
        // const auto u = Vec2<iq16>{6 * x, 0};

        const auto u0 = clock::micros();
        td.update(u);
        const auto u1 = clock::micros();
        DEBUG_PRINTLN(u, td.state()[0][0], td.state()[1][0], td.state()[2], u1 - u0);
    };


    static constexpr uint ISR_FREQ = 20000;

    static constexpr auto mc_w = 20.8_iq12;

    #if 0
    static dsp::CommandShaper1 cs{{
        .kp = mc_w * mc_w,
        .kd = 2 * mc_w,
        .max_spd = 60.0_r,
        // .max_acc = 200.0_r,
        // .max_acc = 120.0_r,
        // .max_acc = 100.0_r,
        // .max_acc = 260.0_r,
        .max_acc = 170.0_r,
        .fs = ISR_FREQ
    }};
    #endif

    static adrc::LinearExtendedStateObserver leso{
        adrc::LinearExtendedStateObserver::Config{
            .b0 = 1,
            .w = mc_w / 3,
            .fs = ISR_FREQ
        }
    };

    static mock::MockBrushedMotor motor{{.fs = ISR_FREQ}};
    // uint32_t exe;
    auto watch_gpio = hal::PC<13>();
    watch_gpio.outpp();
    auto test_leso = [&](const iq16 t){
        // const auto tau = 80.0_r;

        // const auto p0 = 12 * sign(sin(2 * t));
        // const auto p0 = 0.2_r * int(12 * t);
        // const auto p0 = 2 * frac(3 * t);
        // const auto p0 = 12 * frac(t);
        // const auto p0 = 36 * t;
        const auto d = 30 * sign(tpzpu(7 * t));
        // const auto p0 = 12 * tpzpu(t);
        // const auto p0 = CLAMP2(10 * tpzpu(t/4), 5);
        // const auto p0 = 30 * sin(t);

        // const auto u0 = clock::micros();


        // cs.update(p0);
        const iq16 p = 1;
        const iq16 v = 0;


        static constexpr auto mc_w2 = mc_w;
        static constexpr auto kp = mc_w2 * mc_w2;
        static constexpr auto kd = 2 * mc_w2;
        // const auto u = kd * dsp::adrc::ssqrt(p - motor.state()[0]) + kd * (v - motor.state()[1]);
        // const auto u = kp * (p - motor.state()[0]) + kd * (v - motor.state()[1]);
        const auto u = CLAMP2(kp * (p - motor.state()[0]) + kd * (v - motor.state()[1]), 89);
        // const auto dist_inj = + 0.1_r* math::sinpu(3 * t);
        // const auto dist_inj = 80 + 30.1_r * sin(10 * t);
        const auto dist_inj = d;
        

        motor.update(u + dist_inj - leso.disturbance());
        // motor.update(u + dist_inj - leso.disturbance());
        leso.update(motor.state()[1], u);
        // const auto u1 = clock::micros();


        // DEBUG_PRINTLN(
        //     p0, p, v, u,
        //     motor.state()[0], motor.state()[1],
        //     leso.disturbance(),
        //     u1 - u0, dist_inj
        // );

        // exe = u1 - u0;
    };

    clock::delay(20ms);

    real_t t = 0.0_r;

    auto & timer = hal::timer1;
    timer.init({
        .count_freq = hal::NearestFreq(ISR_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }, EN);

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            watch_gpio.set_low();
            t += (1.0_r / ISR_FREQ);
            test_leso(t);
            watch_gpio.set_high();
            break;
        }
        default: break;
        }
    });


    while(true){
        clock::delay(1ms);

        DEBUG_PRINTLN(
            //     p0, p, v, u,
            motor.state()[0], motor.state()[1],
            leso.disturbance()
            //     u1 - u0, dist_inj
        );
    }
}

#endif