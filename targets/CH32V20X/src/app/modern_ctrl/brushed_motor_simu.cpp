#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"

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
        DEBUG_PRINTLN(u, td.state()[0][0], td.state()[1][0], td.state()[2], u1 - u0);
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
    auto watch_gpio = hal::PC<13>();
    watch_gpio.outpp();
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


        cs.update(p0);
        const auto p = cs.state()[0];
        const auto v = cs.state()[1];


        static constexpr auto mc_w2 = mc_w;
        static constexpr auto kp = mc_w2 * mc_w2;
        static constexpr auto kd = 2 * mc_w2;
        // const auto u = kd * dsp::adrc::ssqrt(p - motor.state()[0]) + kd * (v - motor.state()[1]);
        // const auto u = kp * (p - motor.state()[0]) + kd * (v - motor.state()[1]);
        const auto u = CLAMP2(kp * (p - motor.state()[0]) + kd * (v - motor.state()[1]), 89);
        // const auto dist_inj = + 0.1_r* sinpu(3 * t);
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

    hal::timer1.init({ISR_FREQ}, EN);
    hal::timer1.attach<hal::TimerIT::Update>({0,0}, 
        [&]{
            watch_gpio.clr();
            t += (1.0_r / ISR_FREQ);
            test_leso(t);
            watch_gpio.set();
        }, EN
    );

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