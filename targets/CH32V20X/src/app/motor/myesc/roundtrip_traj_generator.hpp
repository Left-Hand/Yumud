#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::motioner{
struct RoundtripParaments{
    uint32_t fs;

    //每个方向需要旋转的圈数
    uint32_t revs_per_direction : 8;

    //转动一周消耗的时刻数
    uint32_t ticks_per_rev : 24;


    int64_t x1_initial;

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
    iiq32 x1;
    iq20 x2;
    iq20 x3;
    uint32_t t_stagelocal;
    RoundtripStage stage;
};

// static_assert(sizeof(RoundtripPollResult) == 20);

static consteval int64_t make_position_from_turns(const float turns){
    return int64_t(static_cast<long double>(turns) * (1ull << 32));
}

struct [[nodiscard]] alignas(size_t) RoundtripTrajGeneratorState final {
    using Self = RoundtripTrajGeneratorState;



    // 常量定义
    static constexpr uint32_t LG2_T_ACC = 12;
    static constexpr uint32_t LG2_K = LG2_T_ACC + 1;
    static constexpr uint32_t T_ACC = 1u << LG2_T_ACC;

    // 1 / 16 圈
    static constexpr uint32_t LEAST_BUFFERING_P64 = 1ll << (32 - 4);

    int64_t p64_0;      // 初始化时位置
    int64_t p64_a;      // 加速结束位置
    int64_t p64_b;      // 缓冲结束位置（正向匀速开始）

    
    uint32_t b;         // 速度系数
    uint32_t fs;        // 采样频率
    uint32_t revs_per_direction;// 每个方向需要旋转的圈数
    
    uint32_t t_buff_outer;   // 外缓冲耗时
    uint32_t t_buff_inner;   // 内缓冲耗时
    uint32_t t_uniform;      // 匀速旋转耗时

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_uniform_x1delta(const uint32_t t_diff) const {
        return int64_t(b) * int64_t(t_diff);
    };

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_accdec_x1delta(const uint32_t t_diff) const {
        const uint64_t squ_t_diff = uint64_t(t_diff) * t_diff;
        return (squ_t_diff * b) >> LG2_K;
    };


    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_uniform_x2() const {
        // 匀速段速度恒定
        return iq20::from_bits((int64_t(b) * fs) >> LG2_T_ACC);
    }

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_accdec_x2(const uint32_t t_diff) const {
        const uint64_t v = uint64_t(b) * t_diff * fs;
        //TODO why 24;
        return iq20::from_bits(int32_t(int64_t(v >> 24)));
    }

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_accdec_x3() const {
        //b * f * f / 2^32 / T_ACC
        static constexpr size_t Q_NUM = 20;
        static constexpr size_t RIGHT_SHIFTS = 32 + LG2_T_ACC - Q_NUM;
        const uint64_t v = uint64_t(b) * fs * fs;
        return iq20::from_bits(int32_t(int64_t(v >> RIGHT_SHIFTS)));
    }


    static constexpr Self from(const RoundtripParaments & para){
        Self self;
        self.init(para);
        return self;
    }

    __attribute__((optimize("-Ofast")))
    constexpr void init(const RoundtripParaments & para) {
        revs_per_direction = para.revs_per_direction;
        b = (UINT32_MAX / uint32_t(para.ticks_per_rev)) + 1;

        // 计算位置
        const int64_t p64_acc = calc_accdec_x1delta(T_ACC);
        p64_0 = para.x1_initial;
        p64_a = para.x1_initial + p64_acc;
        p64_b = ceil_p64(p64_a + LEAST_BUFFERING_P64);
        fs = para.fs;

        // 计算时间参数
        t_buff_outer = (uint64_t(p64_b - p64_a) * para.ticks_per_rev) >> 32;
        t_buff_inner = (uint64_t(LEAST_BUFFERING_P64) * para.ticks_per_rev) >> 32;
        t_uniform = revs_per_direction * para.ticks_per_rev;
    }

    __attribute__((optimize("-Os")))
    constexpr RoundtripPollResult calc_roundtrip_curve(const uint32_t t) const {
        // 计算总时间
        const uint32_t t_acc = T_ACC;
        const uint32_t half_t_total = (t_acc + t_buff_outer + t_uniform + t_buff_inner + t_acc);
        const uint32_t t_total = half_t_total << 1;

        int64_t p64_d = p64_add_revs(p64_b, revs_per_direction);      // 正向匀速结束位置
        int64_t p64_e = p64_d + LEAST_BUFFERING_P64;      // 减速开始位置
        
        // 判断正向还是反向
        const bool is_forward = t < half_t_total;

        const uint32_t t_half = is_forward ? t : (t_total - t);
        
        // 在半个周期内计算位置
        int64_t p64;
        iq20 x2;
        iq20 x3 = 0;
        RoundtripStage stage;
        uint32_t t_stagelocal;
        
        uint32_t t_base = 0;
        if(t >= t_total){
            t_stagelocal = t - t_total;
            p64 = p64_0;
            x2 = 0;
            x3 = 0;
            stage = RoundtripStage::BackwardDeacc;
        } else if (t_base += t_acc; t_half < t_base) {
            // 阶段1: 加速
            t_stagelocal = t_half;
            p64 = p64_0 + calc_accdec_x1delta(t_stagelocal);
            x2 = calc_accdec_x2(t_stagelocal);
            stage = is_forward ? RoundtripStage::InitialAcc : RoundtripStage::BackwardDeacc;
            x3 = calc_accdec_x3();
        } else if (t_base += t_buff_outer; t_half < t_base) {
            // 阶段2: 外缓冲
            t_stagelocal = t_half - t_acc;
            const uint32_t t_reversed = (t_acc + t_buff_outer) - t_half;
            p64 = p64_b - calc_uniform_x1delta(t_reversed);
            x2 = calc_uniform_x2();
            stage = is_forward ? RoundtripStage::ForwardEntryBuffering 
                            : RoundtripStage::BackwardExitBuffering;
            
        } else if (t_base += t_uniform; t_half < t_base) {
            // 阶段3: 匀速
            t_stagelocal = t_half - (t_acc + t_buff_outer);
            p64 = p64_b + calc_uniform_x1delta(t_stagelocal);
            x2 = calc_uniform_x2();
            stage = is_forward ? RoundtripStage::ForwardSpin 
                            : RoundtripStage::BackwardSpin;
            
        } else if (t_base += t_buff_inner; t_half < t_base) {
            // 阶段4: 内缓冲
            t_stagelocal = t_half - (t_acc + t_buff_outer + t_uniform);
            p64 = p64_d + calc_uniform_x1delta(t_stagelocal);
            x2 = calc_uniform_x2();
            stage = is_forward ? RoundtripStage::ForwardExitBuffering 
                            : RoundtripStage::BackwardEntryBuffering;
            
        } else {
            // 阶段5: 减速（包含后半段的反向加速）
            t_stagelocal = t_half - (t_acc + t_buff_outer + t_uniform + t_buff_inner);
            p64 = p64_e + calc_uniform_x1delta(t_stagelocal) 
                - calc_accdec_x1delta(t_stagelocal);

            x2 = calc_uniform_x2() - calc_accdec_x2(t_stagelocal);
            x3 = -calc_accdec_x3();
            stage = is_forward ? RoundtripStage::ForwardDeacc 
                            : RoundtripStage::BackwardAcc;

        }
        

        if(not is_forward){
            x2 = -x2;
        }

        return RoundtripPollResult{
            .x1 = iiq32::from_bits(p64),
            .x2 = x2,
            .x3 = x3,
            .t_stagelocal = t_stagelocal,
            .stage = stage,
        };
    }
private:
    static constexpr int64_t p64_add_revs(int64_t x, int32_t n_revs) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + n_revs) << 32) | frac;
    }

    static constexpr int64_t ceil_p64(int64_t x) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + bool(frac)) << 32);
    }
};
}