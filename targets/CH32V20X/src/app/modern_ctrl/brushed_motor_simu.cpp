#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

using namespace ymd;
using namespace ymd::robots;

void test_burshed_motor(){

    dsp::TdVec2 td{{
        .kp = tau * tau,
        .kd = 2 * tau,
        .max_spd = 60.0_r,
        .max_acc = 1000.0_r,
        .fs = 1000
    }};



    [[maybe_unused]]
    auto test_td = [&](const auto t){
        // const auto u = 6 * Vec2<q16>::RIGHT.rotated(real_t(TAU) * t);
        // const auto [x,y] = sincos(real_t(TAU) * t);
        // const auto m = sin(3 * real_t(TAU) * t);
        // const auto [x,y] = sincos(ret);
        // const auto u = Vec2<q16>{CLAMP(70 * x, -30, 30), 6 * y};
        // const auto u = Vec2<q16>{CLAMP(70 * x, -5, 5) + m, 0};
        // const auto u = Vec2<q16>{10 * frac(t * 3), 0};
        const auto u = Vec2<q16>{10 * frac(t), 0};
        
        // const auto u = Vec2<q16>{CLAMP(70 * x, -30, 30), 0};
        // const auto u = Vec2<q16>{6 * x, 0};

        const auto u0 = clock::micros();
        td.update(u);
        const auto u1 = clock::micros();
        DEBUG_PRINTLN(u, td.get()[0][0], td.get()[1][0], td.get()[2], u1 - u0);
    };

    [[maybe_unused]]
    auto test_cs = [&](const auto t){
        const auto tau = 80.0_r;
        // const auto u = 6 * Vec2<q16>::RIGHT.rotated(real_t(TAU) * t);
        // const auto [x,y] = sincos(real_t(TAU) * t);
        // const auto m = sin(3 * real_t(TAU) * t);
        // const auto [x,y] = sincos(ret);
        // const auto u = Vec2<q16>{CLAMP(70 * x, -30, 30), 6 * y};
        // const auto u = Vec2<q16>{CLAMP(70 * x, -5, 5) + m, 0};
        
        // const auto u = 10 * frac(t);
        // const auto u = 10 * sinpu(t);
        // const auto u = 150 * CLAMP2(sin(t/5), 0.7_r);
        // const auto u = 15 * CLAMP2(sin(t), 0.7_r);
        // const auto u = 15 * t + 5 * sin(3 * t);
        // const auto u = 5 * frac(t);
        const auto u = 10 * sign(sin(3 * t));

        // const auto u = Vec2<q16>{CLAMP(70 * x, -30, 30), 0};
        // const auto u = Vec2<q16>{6 * x, 0};

        static dsp::Leso leso{dsp::Leso::Config{
            .b0 = 1,
            .w = 17.8_r,
            .fs = 1000
        }};

        static dsp::CommandShaper1 cs{{
            .kp = tau * tau,
            .kd = 2 * tau,
            .max_spd = 40.0_r,
            // .max_acc = 200.0_r,
            // .max_acc = 80.0_r,
            .max_acc = 100.0_r,
            .fs = 1000
        }};


        const auto u0 = clock::micros();
        cs.update(u);
        const auto u1 = clock::micros();

        leso.update(cs.get()[0], u);
        DEBUG_PRINTLN(
            u,
            cs.get()[0],
            leso.get_disturbance(),
            u1 - u0
        //     cs.get()[0], 
        //     cs.get()[1], 
        //     cs.get()[2],
        //     cs.kp_,
        //     cs.max_acc_
        //     // u1 - u0
        );
    };

    static constexpr uint ISR_FREQ = 20000;

    static constexpr auto mc_w = 20.8_q12;
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

    static dsp::Leso leso{dsp::Leso::Config{
        .b0 = 1,
        .w = mc_w / 3,
        .fs = ISR_FREQ
    }};

    static mock::MockBrushedMotor motor{{.fs = ISR_FREQ}};
    // uint32_t exe;
    auto & test_gpio = hal::PC<13>();
    test_gpio.outpp();
    auto test_leso = [&](const auto t){
        // const auto tau = 80.0_r;

        // const auto p0 = 12 * sign(sin(2 * t));
        // const auto p0 = 0.2_r * int(12 * t);
        // const auto p0 = 2 * frac(3 * t);
        // const auto p0 = 12 * frac(t);
        const auto p0 = 36 * t;
        const auto d = 30 * sign(tpzpu(7 * t));
        // const auto p0 = 12 * tpzpu(t);
        // const auto p0 = CLAMP2(10 * tpzpu(t/4), 5);
        // const auto p0 = 30 * sin(t);

        // const auto u0 = clock::micros();
        test_gpio.clr();

        cs.update(p0);
        const auto p = cs.get()[0];
        const auto v = cs.get()[1];


        static constexpr auto mc_w2 = mc_w;
        static constexpr auto kp = mc_w2 * mc_w2;
        static constexpr auto kd = 2 * mc_w2;
        // const auto u = kd * dsp::adrc::ssqrt(p - motor.get()[0]) + kd * (v - motor.get()[1]);
        // const auto u = kp * (p - motor.get()[0]) + kd * (v - motor.get()[1]);
        const auto u = CLAMP2(kp * (p - motor.get()[0]) + kd * (v - motor.get()[1]), 89);
        // const auto dist_inj = + 0.1_r* sinpu(3 * t);
        // const auto dist_inj = 80 + 30.1_r * sin(10 * t);
        const auto dist_inj = d;
        

        motor.update(u + dist_inj - leso.get_disturbance());
        // motor.update(u + dist_inj - leso.get_disturbance());
        leso.update(motor.get()[1], u);
        // const auto u1 = clock::micros();
        test_gpio.set();

        // DEBUG_PRINTLN(
        //     p0, p, v, u,
        //     motor.get()[0], motor.get()[1],
        //     leso.get_disturbance(),
        //     u1 - u0, dist_inj
        // );

        // exe = u1 - u0;
    };

    clock::delay(20ms);
    // bindSystickCb([&]{
    //     const auto t = time();
    //     // test_td(t);
    //     test_cs(t);
    // });
    
    real_t t = 0.0_r;

    hal::timer1.init({ISR_FREQ});
    hal::timer1.attach(hal::TimerIT::Update, {0,0}, [&]{
        t += (1.0_r / ISR_FREQ);
        test_leso(t);

    });

    while(true){
        clock::delay(1ms);

        DEBUG_PRINTLN(
            //     p0, p, v, u,
                motor.get()[0], motor.get()[1],
                leso.get_disturbance()
            //     u1 - u0, dist_inj
        );
    }
}