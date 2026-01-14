#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/utils/zero.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "core/string/string_view.hpp"
#include "dsp_lpf.hpp"
using namespace ymd;

static constexpr size_t FOC_FREQ = 16000;
static constexpr auto DT = uq32::from_rcp(FOC_FREQ);

static constexpr size_t PLL_PI_FC = 200;

[[maybe_unused]] static uint32_t generate_noise(){
    static uint32_t seed = 0;
    seed = seed * 214013 + 2531011;
    return seed;
}


struct [[nodiscard]] SinCosCorrector{
    struct Config{

    };
};



void sincospll_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    hal::timer2.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq =  hal::NearestFreq(FOC_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();
    hal::timer2.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    Angular<uq32> simulated_angle_ = Zero;

    iq16 normalized_sine_ = Zero;
    iq16 normalized_cosine_ = Zero;
    
    Angular<iq16> computed_angluar_speed_ = Zero;
    Angular<uq32> computed_angle_ = Zero;

    iq16 err_filtered_ = Zero;
    auto isr_fn = [&]{
        {//simulate input
            [[maybe_unused]] const iq16 now_secs = clock::time();
            const iq16 speed = 50 * iq16(math::sin(now_secs)) + 2 * iq16(math::sin(16 * now_secs));
            // const iq16 speed = 4;
            // const iq16 speed = 2;
            simulated_angle_ = simulated_angle_.from_turns(uq32::from_bits(static_cast<uint32_t>(
                static_cast<int64_t>(simulated_angle_.to_turns().to_bits()) + (
                (static_cast<int64_t>(DT.to_bits()) * speed.to_bits()) >> 16)
            )));
            const auto [sine_, cosine_] = simulated_angle_.sincos();
            const auto [noise_sine_, noise_cosine_] = [] -> std::tuple<iq16, iq16>{
                const uint32_t noise = generate_noise();
                const int32_t i1 = std::bit_cast<int16_t>(static_cast<uint16_t>(noise & 0x07fF));
                const int32_t i2 = std::bit_cast<int16_t>(static_cast<uint16_t>((noise >> 16) & 0x07fF));
                return std::make_tuple(
                    iq16::from_bits(i1),
                    iq16::from_bits(i2)
                );
                // return std::make_tuple(
                //     iq16::from_bits(0),
                //     iq16::from_bits(0)
                // );
            }();

            const auto measured_sine = iq16(sine_) + noise_sine_;
            const auto measured_cosine = iq16(cosine_) + noise_cosine_;

            normalized_sine_ = dsp::lpf_exprimetal(normalized_sine_, measured_sine);
            normalized_cosine_ = dsp::lpf_exprimetal(normalized_cosine_, measured_cosine);
            // normalized_cosine_ = normalized_cosine_ * 0.9_iq16;
            // normalized_cosine_ = normalized_cosine_ + 0.1_iq16;
        }
        
        // static constexpr size_t PLL_PI_FC = 1400;

        // static constexpr size_t PLL_PI_FC = 4;
        static constexpr size_t KP = 2 * PLL_PI_FC;
        static constexpr size_t KI = PLL_PI_FC * PLL_PI_FC;
        static constexpr uq16 KI_BY_FS = uq16::from_bits((KI * (1u << 16)) / FOC_FREQ);

        const auto [sine_, cosine_] = computed_angle_.sincos();
        const iq16 e = ((cosine_) * normalized_sine_- (sine_) * normalized_cosine_);
        err_filtered_ = dsp::lpf_exprimetal(err_filtered_, e);
        // const iq16 e = (simulated_angle_.to_turns() - computed_angle_.to_turns());
        // computed_angluar_speed_ = Angular<iq16>::from_turns(1);
        computed_angluar_speed_ = computed_angluar_speed_.from_turns(
            computed_angluar_speed_.to_turns() + e * KI_BY_FS);

        uint32_t bits = computed_angle_.to_turns().to_bits();
        bits += static_cast<uint32_t>((
            static_cast<uint64_t>(DT.to_bits()) * ((KP * e + computed_angluar_speed_.to_turns()).to_bits())
        ) >> 16);
        computed_angle_ = computed_angle_.from_turns(
            uq32::from_bits(bits)
        );
    };

    hal::timer2.set_event_handler([&](const hal::TimerEvent & event){
        switch(event){
            case hal::TimerEvent::Update:
                isr_fn();
                break;
            default:
                break;
        }
    });

    hal::timer2.start();

    while(true){
        DEBUG_PRINTLN(
            simulated_angle_.to_turns(),
            normalized_sine_, 
            normalized_cosine_,

            computed_angluar_speed_.to_turns(),
            computed_angle_.to_turns(),
            err_filtered_,
            Angular<iq16>::from_turns(iq16(simulated_angle_.to_turns()) - iq16(computed_angle_.to_turns())).signed_normalized().to_turns(),
            // (computed_angle_ + Angular<uq32>::from_turns(0.125_uq32)).unsigned_normalized().to_turns()
            // myiqmath::details::test(normalized_sine_, normalized_cosine_)
            // iq31::from_bits(myiqmath::details::Atan2Intermediate::transfrom_uq31_x_to_uq32_result(simulated_angle_.to_turns().to_bits() >> 1))

            // math::atan2pu(normalized_sine_, normalized_cosine_)
            (computed_angle_ + dsp::calc_lpf_phaseshift_uq32(800, computed_angluar_speed_.to_turns())).unsigned_normalized().to_turns()
            // (computed_angle_ + Angular<uq32>::from_turns(uq32::from_bits(static_cast<int32_t>(static_cast<int64_t>(computed_angluar_speed_.to_turns().to_bits() << 16) / 800)))).unsigned_normalized().to_turns()
            
        );
    }
}