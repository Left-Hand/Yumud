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
    uint32_t revs_per_direction;

    //转动一周消耗的时刻数
    uint32_t ticks_per_rev;
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

struct [[nodiscard]] alignas(size_t) RoundtripPollResult{
    int64_t position;
    iq20 speed;
    uint32_t t_stagelocal : 24;
    RoundtripStage stage : 8;
};

static_assert(sizeof(RoundtripPollResult) == 16);

static consteval int64_t make_position_from_turns(const float turns){
    return int64_t(static_cast<long double>(turns) * (1ull << 32));
}

struct [[nodiscard]] alignas(size_t) RoundtripState final {
    using Self = RoundtripState;



    // 常量定义（保持不变）
    static constexpr uint32_t LG2_T_ACC = 12;
    static constexpr uint32_t LG2_K = LG2_T_ACC + 1;
    static constexpr uint32_t T_ACC = 1u << LG2_T_ACC;
    static constexpr int64_t BUFFERING_POSDIFF = make_position_from_turns(1.0f / 16);

    int64_t p_0;
    int64_t p_a;      // 加速结束位置
    int64_t p_b;      // 缓冲结束位置（正向匀速开始）

    
    uint32_t a;       // 速度系数
    uint32_t revs_per_direction;// 总圈数
    
    // === 精简后的时间参数（仅保留4个） ===
    uint32_t t_buff_outer;   // 外缓冲耗时
    uint32_t t_buff_inner;   // 内缓冲耗时
    uint32_t t_spin;         // 匀速段耗时（fullrevs * t_rev）

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_linear_deltax1(const uint32_t t_diff) const {
        return int64_t(a) * int64_t(t_diff);
    };

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_linear_x2(const uint32_t t_diff) const {
        // 匀速段速度恒定
        // v = a * F_SAMPLE >> 12
        return iq20::from_bits((int64_t(a) * F_SAMPLE) >> LG2_T_ACC);
    }

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_acc_x2(const uint32_t t_diff) const {
        const uint64_t v = uint64_t(t_diff) * a * F_SAMPLE;
        return iq20::from_bits(int32_t(int64_t(v >> 24)));
    }

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_acc_deltax1(const uint32_t t_diff) const {
        const uint64_t squ_t_diff = uint64_t(t_diff) * t_diff;
        return (squ_t_diff * a) >> LG2_K;
    };

    static constexpr Self from(const RoundtripParaments & para){
        Self self;
        self.init(para);
        return self;
    }

    // 优化后的初始化
    __attribute__((optimize("-Os")))
    constexpr void init(const RoundtripParaments & para) {
        revs_per_direction = para.revs_per_direction;
        a = (UINT32_MAX / para.ticks_per_rev) + 1;

        // 计算位置
        const int64_t p_acc = calc_acc_deltax1(T_ACC);
        p_0 = para.p_0;
        p_a = para.p_0 + p_acc;
        p_b = ceil_position(p_a + BUFFERING_POSDIFF);


        // 计算时间参数
        t_buff_outer = ((p_b - p_a) * para.ticks_per_rev) >> 32;
        t_buff_inner = (BUFFERING_POSDIFF * para.ticks_per_rev) >> 32;
        t_spin = revs_per_direction * para.ticks_per_rev;
    }

    // === 性能优化后的轨迹计算 ===
    __no_inline __attribute__((optimize("-Os")))
    constexpr RoundtripPollResult calc_roundtrip_curve(const uint32_t t) const {
        // 计算总时间
        const uint32_t t_acc = T_ACC;
        const uint32_t half_t_total = (t_acc + t_buff_outer + t_spin + t_buff_inner + t_acc);
        const uint32_t t_total = half_t_total << 1;

        int64_t p_d;      // 正向匀速结束位置
        int64_t p_e;      // 减速开始位置
        p_d = position_add_revs(p_b, revs_per_direction);
        p_e = p_d + BUFFERING_POSDIFF;
        
        // 判断正向还是反向
        const bool is_forward = t < half_t_total;

        const uint32_t t_half = is_forward ? t : (t_total - t);
        
        // 在半个周期内计算位置
        int64_t p;
        iq20 speed = 0;
        RoundtripStage stage;
        uint32_t t_stagelocal;
        
        uint32_t t_base = 0;
        if(t >= t_total){
            p = p_0;
            stage = RoundtripStage::BackwardDeacc;
            t_stagelocal = t - t_total;
        } else if (t_base += t_acc; t_half < t_base) {
            // 阶段1: 加速
            t_stagelocal = t_half;
            p = p_0 + calc_acc_deltax1(t_stagelocal);
            speed = calc_acc_x2(t_stagelocal);
            stage = is_forward ? RoundtripStage::InitialAcc : RoundtripStage::BackwardDeacc;
            
        } else if (t_base += t_buff_outer; t_half < t_base) {
            // 阶段2: 外缓冲
            t_stagelocal = t_half - t_acc;
            const uint32_t t_reversed = (t_acc + t_buff_outer) - t_half;
            p = p_b - calc_linear_deltax1(t_reversed);
            speed = calc_linear_x2(t_reversed);
            stage = is_forward ? RoundtripStage::ForwardEntryBuffering 
                            : RoundtripStage::BackwardExitBuffering;
            
        } else if (t_base += t_spin; t_half < t_base) {
            // 阶段3: 匀速
            t_stagelocal = t_half - (t_acc + t_buff_outer);
            p = p_b + calc_linear_deltax1(t_stagelocal);
            speed = calc_linear_x2(t_stagelocal);
            stage = is_forward ? RoundtripStage::ForwardSpin 
                            : RoundtripStage::BackwardSpin;
            
        } else if (t_base += t_buff_inner; t_half < t_base) {
            // 阶段4: 内缓冲
            t_stagelocal = t_half - (t_acc + t_buff_outer + t_spin);
            p = p_d + calc_linear_deltax1(t_stagelocal);
            speed = calc_linear_x2(t_stagelocal);
            stage = is_forward ? RoundtripStage::ForwardExitBuffering 
                            : RoundtripStage::BackwardEntryBuffering;
            
        } else {
            // 阶段5: 减速（包含后半段的反向加速）
            t_stagelocal = t_half - (t_acc + t_buff_outer + t_spin + t_buff_inner);
            if (t_stagelocal < t_acc) {
                // 减速段
                p = p_e + calc_linear_deltax1(t_stagelocal) 
                    - calc_acc_deltax1(t_stagelocal);

                speed = calc_linear_x2(t_stagelocal) - calc_acc_x2(t_stagelocal);
                stage = is_forward ? RoundtripStage::ForwardDeacc 
                                : RoundtripStage::BackwardAcc;
            } else {
                // 反向加速段（实际上已经过了中点）
                const uint32_t t_after_deacc = t_stagelocal - t_acc;
                if (is_forward) {
                    // 正向的后半段：进入反向加速
                    p = p_e + calc_acc_deltax1(t_after_deacc) 
                        - calc_linear_deltax1(t_acc + t_after_deacc);
                    speed = calc_acc_x2(t_after_deacc) - calc_linear_x2(t_acc + t_after_deacc);
                    stage = RoundtripStage::BackwardAcc;
                    t_stagelocal = t_after_deacc;
                } else {
                    // 反向的后半段：回到起点
                    p = p_0;
                    speed = 0;
                    stage = RoundtripStage::BackwardDeacc;
                    t_stagelocal = t_after_deacc;
                }
            }
        }
        

        if(not is_forward){
            speed = -speed;
        }

        return RoundtripPollResult{
            .position = p,
            .speed = speed,
            .t_stagelocal = t_stagelocal,
            .stage = stage
        };
    }
private:
    static constexpr int64_t position_add_revs(int64_t x, int32_t n_revs) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + n_revs) << 32) | frac;
    }

    static constexpr int64_t ceil_position(int64_t x) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + bool(frac)) << 32);
    }
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

    static constexpr uint32_t TICKS_PER_REV = 14 * 6 * 8 * 64;
    // static constexpr float x2_f = (float(F_SAMPLE) / TICKS_PER_REV);
    static constexpr auto RBTRIP_PARAS = RoundtripParaments{
        .p_0 = int64_t(INT32_MIN) * 7, 
        .revs_per_direction = 2, 
        .ticks_per_rev = TICKS_PER_REV
    };
    static constexpr auto roundtrip_state = RoundtripState::from(RBTRIP_PARAS);
    int64_t x1;
    RoundtripStage stage;
    uint32_t t_stagelocal = 0;
    iq20 speed;

    auto isr_fn = [&]{
        static uint32_t t_counter = 0;
        t_counter++;
        const uint32_t t = (t_counter < 20000) ? 0 : (t_counter - 20000);
        auto res = roundtrip_state.calc_roundtrip_curve(t);
        x1 = res.position;
        speed = res.speed;
        stage = res.stage;
        t_stagelocal = res.t_stagelocal;
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
            // iq20::from_bits((int64_t(roundtrip_state.a) * F_SAMPLE) >> 12),
            speed,
            t_stagelocal,
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