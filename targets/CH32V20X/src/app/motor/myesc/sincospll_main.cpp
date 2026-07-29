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


using namespace ymd;



static constexpr size_t F_SAMPLE = 25000;
static constexpr auto DT = uq32::from_rcp(F_SAMPLE);


[[maybe_unused]] static uint32_t generate_noise(){
    static uint32_t seed = 0;
    seed = seed * 214013 + 2531011;
    return seed;
}


struct [[nodiscard]] SinCosCorrector{
    struct Config{

    };
};

// struct alignas(size_t) [[nodiscard]] ShortString8 final{
//     using Self = ShortString8;

//     static constexpr size_t CAPACITY = 8;

//     uint64_t bits;

//     template<size_t N>
//     static constexpr from(const char (&str)[N]){
//         std::array<char, 8> chars;
        
//     }
// };


struct RoundtripParaments{
    int64_t p_0;

    //每个方向需要旋转的圈数
    uint32_t fullrevs;

    //转动一周消耗的时刻数
    uint32_t t_rev;
};


enum class [[nodiscard]] RoundtripStage:uint8_t{
    InitialAcc,
    ForwardEntryBuffering,
    ForwardSpin,
    ForwardExitBuffering,
    ForwardDeacc,
    BackwardAcc,
    BackwardEntryBuffering,
    BackwardSpin,
    BackwardExitBuffering,
    BackwardDeacc,
};

struct [[nodiscard]] RoundtripPollResult{
    int64_t position;
    uint32_t t_sincestage;
    RoundtripStage stage;
};

static consteval int64_t make_position_from_turns(const float turns){
    return int64_t(static_cast<long double>(turns) * (1ull << 32));
}

struct alignas(size_t) [[nodiscard]] RoundtripState final{
    using Self = RoundtripState;



    static constexpr int64_t position_add_revs(int64_t x, int32_t n_revs){
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + n_revs) << 32) | frac;
    }

    static constexpr int64_t ceil_position(int64_t x){
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + bool(frac)) << 32);
    }

    
    //减小定点量化误差 预乘法系数
    static constexpr uint32_t LG2_T_ACC = 12;
    static constexpr uint32_t LG2_K = LG2_T_ACC + 1;
    static constexpr uint32_t T_ACC = 1u << LG2_T_ACC;
    static constexpr int64_t BUFFERING_POSDIFF = make_position_from_turns(1.0f / 16);

    int64_t p_0;
    int64_t p_a;
    int64_t p_b;
    int64_t p_d;
    int64_t p_e;
    
    uint32_t a;
    uint32_t fullrevs;
    uint32_t t_rev;
    uint32_t t_acc;
    uint32_t t_a;//开始匀速前缓冲时刻
    uint32_t t_buff;//缓冲耗时，真正的匀速阶段被夹在两段匀速缓冲阶段中，避免加速度后短时间内引入冲击
    uint32_t t_b;//正向匀速时刻
    uint32_t t_d;//正向匀速结束时刻，开始缓冲
    uint32_t t_e;//正向结束，开始正向减速时刻
    uint32_t t_ee; //逆向前缓冲开始时刻
    uint32_t t_dd;//逆向匀速开始时刻
    uint32_t t_bb; //逆向后缓冲开始时刻
    uint32_t t_aa;//逆向减速开始时刻

    static constexpr Self from(const RoundtripParaments & para){
        Self self;
        self.init(para);
        return self;
    }


    __attribute__((optimize( "-Os" )))
    constexpr int64_t calc_linear_delta_position(const uint32_t t_diff) const {
        return int64_t(a) * int64_t(t_diff);
    };

    __attribute__((optimize( "-Os" )))
    constexpr int64_t calc_acc_delta_position(const uint32_t t_diff) const {
        const uint64_t squ_t_diff = uint64_t(t_diff) * t_diff;
        return (squ_t_diff * a) >> LG2_K;
    };

    __attribute__((optimize( "-Os" )))
    constexpr void init(const RoundtripParaments & para){
        fullrevs = para.fullrevs;
        a = (UINT32_MAX / para.t_rev) + 1;

        const int64_t p_acc = calc_acc_delta_position(T_ACC);

        p_0 = para.p_0;
        p_a = p_0 + p_acc;
        p_b = ceil_position(p_a + BUFFERING_POSDIFF);
        p_d = position_add_revs(p_b, fullrevs);
        p_e = p_d + BUFFERING_POSDIFF;
        

        t_rev = para.t_rev;
        t_acc = T_ACC;//加减速耗时
        t_a = 0 + t_acc;//开始匀速前缓冲时刻
        const uint32_t t_buff_outer = ((p_b - p_a) * para.t_rev) >> 32;//缓冲耗时，真正的匀速阶段被夹在两段匀速缓冲阶段中，避免加速度后短时间内引入冲击
        const uint32_t t_buff_inner = ((BUFFERING_POSDIFF) * para.t_rev) >> 32;

        t_b = t_a + t_buff_outer;//正向匀速时刻
        t_d = t_b + fullrevs * para.t_rev;//正向匀速结束时刻，开始缓冲
        t_e = t_d + t_buff_inner;//正向结束，开始正向减速时刻
        t_ee = t_e + 2 * t_acc;//逆向前缓冲开始时刻
        t_dd = t_ee + t_buff_inner;//逆向匀速开始时刻
        t_bb = t_dd + fullrevs * para.t_rev;//逆向后缓冲开始时刻
        t_aa = t_bb + t_buff_outer;//逆向减速开始时刻
    }

    __no_inline __attribute__((optimize( "-Os" )))
    constexpr RoundtripPollResult calc_roundtrip_curve(const uint32_t t) const {
        #define RETURN_RESULT(_stage)\
        return RoundtripPollResult{\
            .position = p,\
            .t_sincestage = t_sincestage,\
            .stage = _stage\
        };\

        // using enum RoundtripStage;

        if(t < t_ee){
            if(t < t_a){
                const uint32_t t_sincestage = t - 0;
                const auto p = p_0 + calc_acc_delta_position(t_sincestage);
                RETURN_RESULT(RoundtripStage::InitialAcc);
            }else if(t < t_b){
                const uint32_t t_sincestage = t - t_a;
                //从匀速开始的位置反推 避免衔接点不良
                const uint32_t t_reversed = t_b - t;
                const auto p = p_b - calc_linear_delta_position(t_reversed);
                RETURN_RESULT(RoundtripStage::ForwardEntryBuffering);
            }else if(t < t_d){
                const uint32_t t_sincestage = t - t_b;
                const auto p = p_b + calc_linear_delta_position(t_sincestage);
                RETURN_RESULT(RoundtripStage::ForwardSpin);
            }else if(t < t_e){
                const uint32_t t_sincestage = t - t_d;
                const auto p = p_d + calc_linear_delta_position(t_sincestage);
                RETURN_RESULT(RoundtripStage::ForwardExitBuffering);   
            }else{
                uint32_t t_sincestage = t - t_e;
                const auto p = p_e + calc_linear_delta_position(t_sincestage) - calc_acc_delta_position(t_sincestage);
                const auto stage = (t_sincestage < t_acc) ? (RoundtripStage::ForwardDeacc) : (RoundtripStage::BackwardAcc);
                if(t_sincestage > t_acc) t_sincestage -= t_acc;
                RETURN_RESULT(stage);  
            }
        }else{
            if (t < t_dd) {
                const uint32_t t_sincestage = t - t_ee;
                const uint32_t t_reversed = t_dd - t;
                const auto p = p_d + calc_linear_delta_position(t_reversed);
                RETURN_RESULT(RoundtripStage::BackwardEntryBuffering);
            }else if (t < t_bb) {
                const uint32_t t_sincestage = t - t_dd;
                const auto p = p_d - calc_linear_delta_position(t_sincestage);
                RETURN_RESULT(RoundtripStage::BackwardSpin);
            }else if (t < t_aa) {
                const uint32_t t_sincestage = t - t_bb;
                const auto p = p_b - calc_linear_delta_position(t_sincestage);
                RETURN_RESULT(RoundtripStage::BackwardExitBuffering);
            }else {
                const uint32_t t_sincestage = t - t_aa;
                int64_t p;
                if (t_sincestage < t_acc) {
                    p = p_a - calc_linear_delta_position(t_sincestage) + calc_acc_delta_position(t_sincestage);
                } else {
                    p = p_0;
                }
                RETURN_RESULT(RoundtripStage::BackwardDeacc);
            }
        }

        __builtin_abort();

        #undef RETURN_RESULT
    }

private:

};

static constexpr iq16 downcast_position(int64_t x){
    const auto frac = uint32_t(x);
    const auto revs = int32_t(x >> 32);
    return iq16::from_bits((revs << 16) | (frac >> 16));
}

void rbtrip_main(){
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

    static constexpr auto RBTRIP_PARAS = RoundtripParaments{.p_0 = int64_t(INT32_MIN) * 7, .fullrevs = 2, .t_rev = 25000};
    static constexpr auto roundtrip_state = RoundtripState::from(RBTRIP_PARAS);
    int64_t x1;
    RoundtripStage stage;
    uint32_t t_sincestage = 0;

    auto isr_fn = [&]{
        static uint32_t t_counter = 0;
        t_counter++;
        const uint32_t t = (t_counter < 20000) ? 0 : (t_counter - 20000);
        auto res = roundtrip_state.calc_roundtrip_curve(t);
        x1 = res.position;
        stage = res.stage;
        t_sincestage = res.t_sincestage;
    };

    Microseconds isr_elapsed_us_ = 0us;

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
            // iq16(frac) + iq16(revs),
            downcast_position(x1),
            downcast_position(roundtrip_state.p_0),
            downcast_position(roundtrip_state.p_a),
            downcast_position(roundtrip_state.p_b),
            downcast_position(roundtrip_state.p_d),
            downcast_position(roundtrip_state.p_e),
            t_sincestage,
            uint8_t(stage),
            isr_elapsed_us_.count()
        );
    }
}


void sincospll_main(){
    rbtrip_main();
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

            [[maybe_unused]] const auto [noise_sine_, noise_cosine_] = [] -> std::tuple<iq16, iq16>{
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