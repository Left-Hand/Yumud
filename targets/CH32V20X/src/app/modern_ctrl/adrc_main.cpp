#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"
#include "dsp/motor_ctrl/tracking_differentiator.hpp"

#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "core/clock/time.hpp"


using namespace ymd;
using namespace ymd::dsp;

static constexpr auto UART_BAUD = 576000;



static constexpr size_t ISR_FREQ = 20000;
void adrc_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    // const auto tau = 80.0_r;

    // static dsp::Leso leso{dsp::Leso::Config{
    //     .b0 = 1,
    //     .w = 17.8_r,
    //     .fs = 1000
    // }};

    // static dsp::CommandShaper1 command_shaper_{{
    //     .kp = tau * tau,
    //     .kd = 2 * tau,
    //     .max_spd = 40.0_r,
    //     // .max_acc = 200.0_r,
    //     // .max_acc = 80.0_r,
    //     .max_acc = 100.0_r,
    //     .fs = 1000
    // }};

    static constexpr auto coeffs = typename NonlinearTrackingDifferentor::Config{
        .fs = ISR_FREQ ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        .r = 256.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 200
    }.try_to_coeffs().unwrap();
    static NonlinearTrackingDifferentor command_shaper_{
        coeffs
    };

    iq16 u = 0;
    Microseconds elapsed_micros = 0us;
    SecondOrderState shaped_track_state_;
    SecondOrderState feedback_track_;


    static constexpr auto track_coeffs = LinearTrackingDifferentiator<iq16, 2>::Config{
        .fs = ISR_FREQ , .r = 140
    }.try_to_coeffs().unwrap();

    [[maybe_unused]] LinearTrackingDifferentiator<iq16, 2> feedback_differ_{
        track_coeffs
    };

    [[maybe_unused]]
    auto command_shaper_poller = [&](){
        const auto ctime = clock::time();
        const auto u0 = clock::micros();
        shaped_track_state_ = command_shaper_.update(shaped_track_state_, u);
        // feedback_track_ = feedback_differ_.update(feedback_track_, iq16::from_bits(shaped_track_state_.x1.to_bits() >> 16));
        // feedback_track_ = feedback_differ_.update(feedback_track_, sin(ctime * 140));
        feedback_track_ = feedback_differ_.update(feedback_track_, frac(ctime));
        // feedback_track_ = feedback_differ_.update(feedback_track_, u);
        const auto u1 = clock::micros();
        elapsed_micros = u1 - u0;

        // leso.update(command_shaper_.state()[0], u);

    };

    auto & timer = hal::timer1;
    timer.init({
        .count_freq = hal::NearestFreq(ISR_FREQ ),
        .count_mode = hal::TimerCountMode::Up
    }, EN);


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
            case hal::TimerEvent::Update:{
                command_shaper_poller();
                break;
            }
            default: break;
        }
    });

    while(true){
        const auto ctime = clock::time();
        // u = -180 + 10 * sign(iq16(sinpu(ctime * 0.5_r)));
        u = -180 + 10 * iq16(sinpu(ctime * 0.5_r));
        // u = 10 * sign(iq16(sinpu(ctime * 0.5_r)));

        DEBUG_PRINTLN(
            u,
            iq16::from_bits(shaped_track_state_.x1.to_bits() >> 16),
            shaped_track_state_.x2,
            iq16::from_bits(feedback_track_.x1.to_bits() >> 16),
            feedback_track_.x2,
            elapsed_micros.count()
            // leso.get_disturbance()
        );

        clock::delay(2ms);
    }

}