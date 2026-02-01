#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/utils/zero.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "dsp_lpf.hpp"
using namespace ymd;

static constexpr size_t F_SAMPLE = 16000;
static constexpr auto DT = uq32::from_rcp(F_SAMPLE);

// static constexpr size_t PLL_PI_FC = 1200;
// static constexpr size_t PLL_PI_FC = 16000;
// static constexpr size_t PLL_PI_FC = 1600;
static constexpr size_t PLL_PI_FC = 1800;
// static constexpr size_t PLL_PI_FC = 900;
// static constexpr size_t PLL_LPF_FC = 1100;
static constexpr size_t PLL_LPF_FC = 4000;
// static constexpr size_t PLL_LPF_FC = 100;
// static constexpr size_t PLL_PI_FC = 100;
// static constexpr size_t PLL_LPF_FC = 100;
// static constexpr size_t PLL_LPF_FC = 300;
static constexpr size_t PLL_KP = 2 * PLL_PI_FC;
static constexpr size_t PLL_KI = PLL_PI_FC * PLL_PI_FC;
static constexpr uq16 PLL_KI_BY_FS = uq16::from_bits(
    static_cast<uint32_t>((static_cast<uint64_t>(PLL_KI) * (1u << 16)) / F_SAMPLE)
);

static constexpr math::fixed_t<32, uint32_t> uq32_mul(const math::fixed_t<32, uint32_t> a, const size_t b){
    const auto bits = static_cast<uint32_t>((static_cast<uint64_t>(a.to_bits()) * b) & std::numeric_limits<uint32_t>::max());
    return math::fixed_t<32, uint32_t>::from_bits(bits);
}
static_assert(uq32_mul(0.125_uq32,13) == 0.625_uq32);

[[maybe_unused]] static uint32_t generate_noise(){
    static uint32_t seed = 0;
    seed = seed * 214013 + 2531011;
    return seed;
}


struct [[nodiscard]] SinCosCorrector{
    struct Config{

    };
};

//a * d - b * c
template<size_t Q1, typename D1, size_t Q2, typename D2, typename ED = tmp::extended_mul_underlying_t<D1, D2>>
static constexpr math::fixed_t<Q1, D1> cross2v2(
    const math::fixed_t<Q1, D1> & a, const math::fixed_t<Q2, D2> & b,
    const math::fixed_t<Q1, D1> & c, const math::fixed_t<Q2, D2> & d
){
    ED bits = 0;
    bits += static_cast<ED>(a.to_bits()) * static_cast<ED>(d.to_bits());
    bits -= static_cast<ED>(b.to_bits()) * static_cast<ED>(c.to_bits());
    return math::fixed_t<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}


void sincospll_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(576_KHz),
        .baudrate = hal::NearestFreq(6000000),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.set_eps(6);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    hal::timer2.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq =  hal::NearestFreq(F_SAMPLE),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();
    hal::timer2.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    Angular<uq32> simulated_angle_ = Zero;

    iq16 normalized_sine_ = Zero;
    iq16 normalized_cosine_ = Zero;

    iq16 measured_sine_ = Zero;
    iq16 measured_cosine_ = Zero;
    
    Angular<iq16> computed_angluar_speed_ = Zero;
    Angular<uq32> computed_angle_ = Zero;

    iq16 err_filtered_ = Zero;
    Microseconds isr_elapsed_us_ = 0us;
    [[maybe_unused]] static constexpr uq32 LPF_ALPHA = dsp::calc_lpf_alpha_uq32(F_SAMPLE, PLL_LPF_FC).unwrap();
    [[maybe_unused]] static constexpr auto LPF_ALPHA_F = float(LPF_ALPHA);
    auto isr_fn = [&]{
        for(size_t i = 0; i < 1; i++){//simulate input
        // if(false){//simulate input
            static uq32 now_secs = 0;
            now_secs += uq32::from_rcp(F_SAMPLE);
            // const iq16 angular_speed = 450 * iq16(math::sinpu(now_secs)) + 164 * iq16(math::sinpu(32 * now_secs));
            // const iq16 angular_speed = 450 * iq16(math::sinpu(now_secs)) + 64 * iq16(math::sinpu(32 * now_secs));
            const iq16 angular_speed = 1450 * iq16(math::sinpu(now_secs)) + 64 * iq16(math::sinpu(10 * now_secs));
            // const iq16 angular_speed = 45 * iq16(math::sinpu(now_secs));
            // const iq16 angular_speed = 4;
            // const iq16 angular_speed = 2;
            simulated_angle_ = simulated_angle_.from_turns(uq32::from_bits(static_cast<uint32_t>(
                static_cast<int64_t>(simulated_angle_.to_turns().to_bits()) + (
                (static_cast<int64_t>(DT.to_bits()) * angular_speed.to_bits()) >> 16)
            )));

            #if 1
            const auto [sine_, cosine_] = simulated_angle_.sincos();
            #else
            const auto sine_= simulated_angle_.sin();
            const auto cosine_= (simulated_angle_ + (Angular<uq32>::from_turns(uq32(0.33333333)))).sin();
            #endif

            [[maybe_unused]] const auto [noise_sine_, noise_cosine_] = [] -> std::tuple<iq16, iq16>{
                const uint32_t noise = generate_noise();
                const int32_t i1 = std::bit_cast<int16_t>(static_cast<uint16_t>(noise & 0x1fF));
                const int32_t i2 = std::bit_cast<int16_t>(static_cast<uint16_t>((noise >> 16) & 0x1fF));
                return std::make_tuple(
                    iq16::from_bits(i1),
                    iq16::from_bits(i2)
                );
                // return std::make_tuple(
                //     iq16::from_bits(0),
                //     iq16::from_bits(0)
                // );
            }();

            measured_sine_ = iq16(sine_);
            measured_cosine_ = iq16(cosine_);
            measured_sine_ += noise_sine_;
            measured_cosine_ += noise_cosine_;


            // normalized_cosine_ = normalized_cosine_ * 0.9_iq16;
            // normalized_cosine_ = normalized_cosine_ + 0.1_iq16;
        }
        
        for(size_t i = 0; i < 1; i++){
            normalized_sine_ = dsp::lpf_with_given_alpha(normalized_sine_, measured_sine_, LPF_ALPHA);
            // normalized_cosine_ = dsp::lpf_with_given_alpha(normalized_cosine_,
            //     measured_cosine_ * iq16(2 / 1.73) + measured_sine_ * iq16(1.0 / 1.73), 
            //     LPF_ALPHA
            // );
            normalized_cosine_ = dsp::lpf_with_given_alpha(normalized_cosine_,
                measured_cosine_,
                LPF_ALPHA
            );
            


            const auto [sine_, cosine_] = computed_angle_.sincos();
            
            // const iq16 e = cross2v2(cosine_ , normalized_cosine_, sine_ , normalized_sine_);
            const iq16 e = cross2v2(normalized_sine_, sine_ , normalized_cosine_, cosine_ );
            // const iq16 e = (iq16(cosine_) * normalized_sine_- iq16(sine_) * normalized_cosine_);
            err_filtered_ = dsp::lpf_with_given_alpha(err_filtered_, e, LPF_ALPHA);
            // const iq16 e = (simulated_angle_.to_turns() - computed_angle_.to_turns());
            // computed_angluar_speed_ = Angular<iq16>::from_turns(1);
            computed_angluar_speed_ = computed_angluar_speed_.from_turns(
                computed_angluar_speed_.to_turns() + e * PLL_KI_BY_FS);

            uint32_t angle_bits = computed_angle_.to_turns().to_bits();
            angle_bits += static_cast<uint32_t>((
                static_cast<uint64_t>(DT.to_bits()) * ((PLL_KP * e + computed_angluar_speed_.to_turns()).to_bits())
            ) >> 16);
            computed_angle_ = computed_angle_.from_turns(
                uq32::from_bits(angle_bits)
            );
        }
    };

    hal::timer2.set_event_callback([&](const hal::TimerEvent & event){
        switch(event){
            case hal::TimerEvent::Update:{
                const auto begin_us = clock::micros();
                isr_fn();
                isr_elapsed_us_ = clock::micros() - begin_us;
                break;
            }
            default:
                break;
        }
    });

    hal::timer2.start();

    while(true){
        DEBUG_PRINTLN(
            clock::seconds(),
            // uq32::from_bits(clock::seconds().to_bits()),
            // uq32::from_bits(clock::seconds().to_bits() >> 32),
            // static_cast<uint32_t>(clock::seconds().to_bits()),

            simulated_angle_.to_turns(),
            computed_angle_.to_turns(),
            computed_angluar_speed_.to_turns(),

            // computed_angluar_speed_.to_turns(),
            // err_filtered_,
            Angular<iq16>::from_turns(iq16(simulated_angle_.to_turns()) - iq16(computed_angle_.to_turns())).signed_normalized().to_turns(),
            // math::pu_to_uq32(math::atan2pu(measured_sine_, measured_cosine_))
            // (computed_angle_ + Angular<uq32>::from_turns(0.125_uq32)).unsigned_normalized().to_turns()
            // myiqmath::details::test(normalized_sine_, normalized_cosine_)
            // iq31::from_bits(myiqmath::details::Atan2Intermediate::transfrom_uq31_x_to_uq32_result(simulated_angle_.to_turns().to_bits() >> 1))

            // math::atan2pu(normalized_sine_, normalized_cosine_)
            (computed_angle_ + dsp::calc_lpf_phaseshift_uq32(PLL_LPF_FC, computed_angluar_speed_.to_turns())).unsigned_normalized().to_turns(),
            // (computed_angle_ + Angular<uq32>::from_turns(uq32::from_bits(static_cast<int32_t>(static_cast<int64_t>(computed_angluar_speed_.to_turns().to_bits() << 16) / 800)))).unsigned_normalized().to_turns()


            // normalized_sine_, 
            // normalized_cosine_,
            measured_sine_, 
            measured_cosine_,
            isr_elapsed_us_.count()

        );
    }
}