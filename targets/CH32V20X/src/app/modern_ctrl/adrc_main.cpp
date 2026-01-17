#include "src/testbench/tb.h"
#include "dsp/controller/adrc/linear/leso2o.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"

#include "middlewares/rpc/repl_server.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "core/clock/time.hpp"


using namespace ymd;
using namespace ymd::dsp;
using namespace ymd::dsp::adrc;

static constexpr auto UART_BAUD = 6000000;

static constexpr size_t ISR_FREQ = 25000;


void adrc_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(UART_BAUD),
            .tx_strategy = CommStrategy::Blocking
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(6);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    static constexpr auto coeffs = typename NonlinearTrackingDifferentiator<iq16, 2>::Config{
        .fs = ISR_FREQ ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 256.5_iq10,
        .r = 556.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 200
    }.try_into_coeffs().unwrap();

    static constexpr auto track_coeffs = typename LinearTrackingDifferentiator<iq16, 2>::Config{
        .fs = ISR_FREQ , .r = 1400
    }.try_into_coeffs().unwrap();

    static constexpr NonlinearTrackingDifferentiator<iq16, 2> command_shaper_{
        coeffs
    };

    iq16 u = 0;
    Microseconds elapsed_micros = 0us;
    SecondOrderState<iq16> shaped_track_state_var_;
    SecondOrderState<iq16> feedback_track_state_var_;




    [[maybe_unused]] LinearTrackingDifferentiator<iq16, 2> feedback_differ_{
        track_coeffs
    };

    [[maybe_unused]]
    auto command_shaper_poller = [&](){
        const auto now_secs = clock::seconds();
        const auto t0 = clock::micros();
        shaped_track_state_var_ = command_shaper_.iterate(shaped_track_state_var_, {u, 0});
        // feedback_track_state_var_ = feedback_differ_.iterate(feedback_track_state_var_, iq16::from_bits(shaped_track_state_var_.x1.to_bits() >> 16));
        // feedback_track_state_var_ = feedback_differ_.iterate(feedback_track_state_var_, sin(now_secs * 140));
        feedback_track_state_var_ = feedback_differ_.iterate(feedback_track_state_var_, {math::frac(now_secs), 1});
        // feedback_track_state_var_ = feedback_differ_.iterate(feedback_track_state_var_, u);
        const auto t1 = clock::micros();
        elapsed_micros = t1 - t0;

        // leso.iterate(command_shaper_.state()[0], u);

    };

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        .count_freq = hal::NearestFreq(ISR_FREQ ),
        .count_mode = hal::TimerCountMode::Up
    })
        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();


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

    timer.start();
    while(true){
        const auto now_secs = clock::seconds();
        u = -180 + 10 * sign(iq16(math::sinpu(now_secs * 0.5_r)));
        // u = now_secs / 1000;
        // u = -180 + CLAMP(10 * iq16(math::sinpu(now_secs * 0.5_r)), -5, 5);
        // u = 10 * sign(iq16(math::sinpu(now_secs * 0.5_r)));

        DEBUG_PRINTLN(
            u,
            iq16::from_bits(shaped_track_state_var_.x1.to_bits() >> 16),
            shaped_track_state_var_.x2,
            iq16::from_bits(feedback_track_state_var_.x1.to_bits() >> 16),
            feedback_track_state_var_.x2,
            elapsed_micros.count()
            // leso.get_disturbance()
        );

        // clock::delay(2ms);
    }

}