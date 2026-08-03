#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/utils/zero.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "motor_dsp/dsp_lpf.hpp"
#include "motor_dsp/dsp_vec.hpp"
#include "motor_dsp/dsp_pll.hpp"
#include "utilities/roundtrip_traj_generator.hpp"


using namespace ymd;
using namespace ymd::motioner;



static constexpr size_t F_SAMPLE = 25000;
static constexpr auto DT = uq32::from_rcp(F_SAMPLE);


struct NoiseGenerator{
    uint32_t seed;

    static constexpr NoiseGenerator from_default(){
        return NoiseGenerator{0};
    }

    constexpr uint32_t next() noexcept {
        seed = seed * 214013 + 2531011;
        return seed;
    }
};

void rbtrip_main();


void sincospll_main(){
    return rbtrip_main();
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        // .baudrate = hal::NearestFreq(6000000),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();


    hal::timer2.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq =  hal::NearestFreq(F_SAMPLE),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();
    hal::timer2.register_nvic<hal::TimerIT::Update>(hal::NvicPriorityCode::highest(),  EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    iq16 mock_angular_speed = 0;
    Angular<uq32> mock_angle_ = Zero;
    Angular<uq32> sync_angle_ = Zero;
    iq16 angular_speed_lp = 0;

    iq16 normalized_sine_ = Zero;
    iq16 normalized_cosine_ = Zero;

    iq16 measured_sine_ = Zero;
    iq16 measured_cosine_ = Zero;
    





    dsp::PllState pll_state_;
    pll_state_.reset();

    static constexpr auto PLL_COEFFS = dsp::PllCoeffs::from_fsfc(F_SAMPLE, 70, 1.7_iq16);
    auto noise_generator_ = NoiseGenerator::from_default();

    auto generate_noise = [&]() -> uint32_t{
        return noise_generator_.next();
    };

    Microseconds isr_elapsed_us_ = 0us;
    // [[maybe_unused]] static constexpr uq32 LPF_ALPHA = dsp::calc_lpf_alpha_uq32(F_SAMPLE, PLL_LPF_FC).unwrap();
    // [[maybe_unused]] static constexpr auto LPF_ALPHA_F = float(LPF_ALPHA);
    auto isr_fn = [&]{
        for(size_t i = 0; i < 1; i++){//simulate input
        // if(false){//simulate input
            static uq32 now_secs = 0;
            now_secs += uq32::from_rcp(F_SAMPLE);
            // const iq16 mock_angular_speed = 450 * iq16(math::sinpu(now_secs)) + 14 * iq16(math::sinpu(32 * now_secs));
            // const iq16 mock_angular_speed = 45;
            // const iq16 mock_angular_speed = 450 * iq16(math::sinpu(now_secs)) + 64 * iq16(math::sinpu(32 * now_secs));
            // mock_angular_speed = 1450 * iq16(math::sinpu(now_secs)) + 64 * iq16(math::sinpu(10 * now_secs));
            mock_angular_speed = 645 * iq16(math::sinpu(now_secs)) + 8 * iq16(math::sinpu(10 * now_secs));
            if(mock_angular_speed < 120) mock_angular_speed = 0;
            // const iq16 mock_angular_speed = 45 * iq16(math::sinpu(now_secs));
            // const iq16 mock_angular_speed = 4;
            // const iq16 mock_angular_speed = 2;
            mock_angle_ = mock_angle_.from_turns(uq32::from_bits(static_cast<uint32_t>(
                static_cast<int64_t>(mock_angle_.to_turns().to_bits()) + (
                (static_cast<int64_t>(DT.to_bits()) * mock_angular_speed.to_bits()) >> 16)
            )));



            #if 1
            const auto [mock_sine, mock_cosine] = mock_angle_.sincos();
            #else
            const auto mock_sine= mock_angle_.sin();
            const auto mock_cosine= (mock_angle_ + (Angular<uq32>::from_turns(uq32(0.33333333)))).sin();
            #endif

            [[maybe_unused]] const auto [noise_sine_, noise_cosine_] = [&] -> std::tuple<iq16, iq16>{
                const uint32_t noise = generate_noise();
                const int32_t i1 = std::bit_cast<int16_t>(static_cast<uint16_t>(noise & 0x1ffF));
                const int32_t i2 = std::bit_cast<int16_t>(static_cast<uint16_t>((noise >> 16) & 0x1ffF));
                return std::make_tuple(
                    iq16::from_bits(i1),
                    iq16::from_bits(i2)
                );
                // return std::make_tuple(
                //     iq16::from_bits(0),
                //     iq16::from_bits(0)
                // );
            }();

            measured_sine_ = iq16(mock_sine);
            measured_cosine_ = iq16(mock_cosine);
            measured_sine_ += noise_sine_;
            measured_cosine_ += noise_cosine_;


            // normalized_cosine_ = normalized_cosine_ * 0.9_iq16;
            // normalized_cosine_ = normalized_cosine_ + 0.1_iq16;
        }

        {
            normalized_sine_ = dsp::lpf_1o(normalized_sine_, measured_sine_, std::numeric_limits<uq32>::max());
            // normalized_cosine_ = dsp::lpf_1o(normalized_cosine_,
            //     measured_cosine_ * iq16(2 / 1.73) + measured_sine_ * iq16(1.0 / 1.73), 
            //     LPF_ALPHA
            // );
            normalized_cosine_ = dsp::lpf_1o(normalized_cosine_,
                measured_cosine_,
                std::numeric_limits<uq32>::max()
            );
            
            PLL_COEFFS.iterate(pll_state_, {normalized_sine_, normalized_cosine_});

            static constexpr auto alpha = dsp::calc_lpf_alpha_uq32(F_SAMPLE, 70).unwrap();
            angular_speed_lp = dsp::lpf_1o(angular_speed_lp, pll_state_.angluar_speed.to_turns(), alpha);
            sync_angle_ = sync_angle_.from_turns(uq32::from_bits(static_cast<uint32_t>(
                static_cast<int64_t>(sync_angle_.to_turns().to_bits()) + (
                (static_cast<int64_t>(DT.to_bits()) * pll_state_.angluar_speed.to_turns().to_bits()) >> 16)
            )));
        }
    };

    hal::timer2.set_isr_callback([&](const hal::TimerEvent & event){
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
            // clock::seconds(),
            // uq32::from_bits(clock::seconds().to_bits()),
            // uq32::from_bits(clock::seconds().to_bits() >> 32),
            // static_cast<uint32_t>(clock::seconds().to_bits()),

            mock_angle_.to_turns(),
            mock_angular_speed,

            pll_state_.angle.to_turns(),
            pll_state_.angluar_speed.to_turns(),
            pll_state_.angluar_speed_integral.to_turns(),

            // pll_state_.angluar_speed.to_turns(),
            // pll_state_.err_filtered,

            Angular<iq16>::from_turns(iq16(mock_angle_.to_turns()) - iq16(pll_state_.angle.to_turns())).signed_normalized().to_turns(),
            sync_angle_.to_turns(),
            measured_sine_,
            measured_cosine_,
            angular_speed_lp,
            // math::pu_to_uq32(math::atan2pu(measured_sine_, measured_cosine_))
            // (pll_state_.angle + Angular<uq32>::from_turns(0.125_uq32)).unsigned_normalized().to_turns()
            // math::atan2pu(normalized_sine_, normalized_cosine_)
            // (pll_state_.angle + dsp::calc_lpf_phaseshift_uq32(PLL_FRONT_LPF_FC, pll_state_.angluar_speed.to_turns())).unsigned_normalized().to_turns(),
            // (pll_state_.angle + Angular<uq32>::from_turns(uq32::from_bits(static_cast<int32_t>(static_cast<int64_t>(pll_state_.angluar_speed.to_turns().to_bits() << 16) / 800)))).unsigned_normalized().to_turns()


            // normalized_sine_, 
            // normalized_cosine_,
            // measured_sine_, 
            // measured_cosine_,
            isr_elapsed_us_.count()

        );
    }
}




static constexpr iq16 downcast_position(int64_t x){
    const auto frac = uint32_t(x);
    const auto revs = int32_t(x >> 32);
    return iq16::from_bits((revs << 16) | (frac >> 16));
}

[[maybe_unused]] void rbtrip_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        // .baudrate = hal::NearestFreq(6000000),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.build_config()
        .set_eps(5)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();


    hal::timer2.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq =  hal::NearestFreq(F_SAMPLE),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();
    hal::timer2.register_nvic<hal::TimerIT::Update>(hal::NvicPriorityCode::highest(),  EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    // static constexpr float x2_f = (float(F_SAMPLE) / TICKS_PER_REV);
    // static constexpr float acc_dt = float(1u << 12) / F_SAMPLE;
    // static constexpr float x3_f = x2_f / acc_dt;
    // static constexpr float x3_f = float(F_SAMPLE) * F_SAMPLE / (1 << 12) / TICKS_PER_REV;
    // static constexpr uint32_t b = (1ull << 32) / TICKS_PER_REV;
    // static constexpr auto x3_iq = iq20::from_bits((int64_t(b) * F_SAMPLE * F_SAMPLE) >> 24);
    // static constexpr auto x3_iq_f = float(x3_iq);
    static constexpr uint32_t TICKS_PER_REV = 14 * 6 * 8 * 4;
    static constexpr auto RBTRIP_PARAS = RoundtripParaments{
        .fs = F_SAMPLE,
        .uniform_ticks = -int32_t(TICKS_PER_REV) * 4, 
        .ticks_per_rev = TICKS_PER_REV,
        .x1_initial = iiq32(-7), 
    };

    static constexpr auto ROUNDTRIP_GEN = RoundtripTrajGenerator::from(RBTRIP_PARAS);
    iiq32 x1;
    RoundtripStage stage;
    uint32_t t_stagelocal = 0;
    iq20 x2;
    iq20 x3;

    auto isr_fn = [&]{
        static uint32_t t_counter = 0;
        t_counter++;
        const uint32_t t = (t_counter < 20000) ? 0 : (t_counter - 20000);
        auto res = ROUNDTRIP_GEN.sample_tick(t);
        x1 = res.x1;
        x2 = res.x2;
        x3 = res.x3;
        stage = res.stage;
        t_stagelocal = res.t_stagelocal;
    };

    Microseconds isr_elapsed_us_ = 0us;

    hal::timer2.set_isr_callback([&](const hal::TimerEvent & event){
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
            // iq16(frac) + iq16(revs),
            downcast_position(x1.to_bits()),
            // iq20::from_bits((int64_t(ROUNDTRIP_GEN.b) * F_SAMPLE) >> 12),
            x2,
            x3,
            downcast_position(ROUNDTRIP_GEN.p64_initial),
            downcast_position(ROUNDTRIP_GEN.p64_entry_uniform),
            // t_stagelocal,
            // uint8_t(stage),
            isr_elapsed_us_.count()
        );
    }
}