#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::motioner{



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

struct [[nodiscard]] alignas(size_t) RoundtripSamplePoint final{
    iiq32 x1;
    iq20 x2;
    iq20 x3;
    uint32_t t_stagelocal;
    RoundtripStage stage;
};

// static_assert(sizeof(RoundtripSamplePoint) == 20);

static consteval int64_t make_position_from_turns(const float turns){
    return int64_t(static_cast<long double>(turns) * (1ull << 32));
}


struct RoundtripParaments{
    uint32_t fs;

    //需要匀速旋转的时刻数 符号表示梯形朝向的正反
    int32_t uniform_ticks;

    //转动一周消耗的时刻数
    uint32_t ticks_per_rev;


    iiq32 x1_initial;

};

struct [[nodiscard]] alignas(size_t) RoundtripTrajGenerator final {
    using Self = RoundtripTrajGenerator;

    // 常量定义
    static constexpr uint32_t LG2_T_ACC = 12;
    static constexpr uint32_t LG2_K = LG2_T_ACC + 1;
    static constexpr uint32_t T_ACC = 1u << LG2_T_ACC;

    // 1 / 16 圈
    static constexpr uint32_t LEAST_BUFFERING_P64 = 1ll << (32 - 4);

    int64_t p64_initial;      // 初始化时位置
    int64_t p64_entry_uniform;      // 缓冲结束位置（正向匀速开始）
    int64_t sp64_uniform;

    struct FactorB{
        int32_t count;
    };

    int32_t b;         // 速度系数


    uint32_t fs;        // 采样频率
    
    uint32_t t_buff_outer_elapsed;   // 外缓冲耗时
    uint32_t t_buff_inner_elapsed;   // 内缓冲耗时
    uint32_t t_uniform_elapsed;      // 匀速旋转耗时

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_uniform_x1delta(const uint32_t t_stagelocal) const {
        return int64_t(b) * int64_t(t_stagelocal);
    };

    __attribute__((optimize( "-Ofast" )))
    constexpr int64_t calc_accdec_x1delta(const uint32_t t_stagelocal) const {
        const uint64_t squ_t_diff = uint64_t(t_stagelocal) * t_stagelocal;
        return (int64_t(squ_t_diff) * int64_t(b)) >> LG2_K;
    };


    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_uniform_x2() const {
        // 匀速段速度恒定
        return iq20::from_bits((int64_t(b) * fs) >> LG2_T_ACC);
    }

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_accdec_x2(const uint32_t t_stagelocal) const {
        const int64_t v = int64_t(b) * t_stagelocal * fs;
        //TODO why 24;
        static constexpr size_t RIGHT_SHIFTS = 24;
        return iq20::from_bits(int32_t(int64_t(v >> RIGHT_SHIFTS)));
    }

    __attribute__((optimize("-Ofast")))
    constexpr iq20 calc_accdec_x3() const {
        //b * f * f / 2^32 / T_ACC
        static constexpr size_t Q_NUM = 20;
        static constexpr size_t RIGHT_SHIFTS = 32 + LG2_T_ACC - Q_NUM;
        const int64_t v = int64_t(b) * fs * fs;
        return iq20::from_bits(int32_t(int64_t(v >> RIGHT_SHIFTS)));
    }


    static constexpr Self from(const RoundtripParaments & para){
        Self self;
        self.init(para);
        return self;
    }

    __attribute__((optimize("-Ofast")))
    constexpr void init(this Self & self, const RoundtripParaments & para) {
        const bool is_inverted = para.uniform_ticks < 0;
        const uint32_t abs_b = _rcp_u32(para.ticks_per_rev);
        self.b = _conditional_neg(is_inverted, int32_t(abs_b));

        const int64_t p64_accdec = self.calc_accdec_x1delta(T_ACC);

        self.p64_initial = para.x1_initial.to_bits();
        const int64_t p64_entry_buffering = para.x1_initial.to_bits() + p64_accdec;
        self.p64_entry_uniform = is_inverted ? 
            _floor_p64(p64_entry_buffering - int64_t(LEAST_BUFFERING_P64)) : 
            _ceil_p64(p64_entry_buffering + int64_t(LEAST_BUFFERING_P64)) ;
        self.sp64_uniform = int64_t(para.uniform_ticks) * abs_b;
        self.fs = para.fs;

        // 计算时间参数
        self.t_buff_outer_elapsed = (_abs_diff(self.p64_entry_uniform, p64_entry_buffering) * para.ticks_per_rev) >> 32;
        self.t_buff_inner_elapsed = (uint64_t(LEAST_BUFFERING_P64) * para.ticks_per_rev) >> 32;
        self.t_uniform_elapsed = _abs(para.uniform_ticks);
    }

    __attribute__((optimize("-Os")))
    constexpr RoundtripSamplePoint sample_tick(const uint32_t abs_t) const {
        // 计算总时间
        bool is_inverted = std::signbit(sp64_uniform);
        const uint32_t t_acc = T_ACC;
        const uint32_t half_t_total = (t_acc + t_buff_outer_elapsed + t_uniform_elapsed + t_buff_inner_elapsed + t_acc);
        const uint32_t t_total = half_t_total << 1;

        int64_t p64_exit_uniform = p64_entry_uniform + sp64_uniform;      // 正向匀速结束位置
        int64_t p64_exit_buffering = p64_exit_uniform + _conditional_neg(is_inverted, int64_t(LEAST_BUFFERING_P64));      // 减速开始位置
        
        // 判断位移绝对值是否在变大
        const bool is_growup = abs_t < half_t_total;

        const uint32_t t = is_growup ? abs_t : (t_total - abs_t);
        
        // 在半个周期内计算位置
        int64_t p64;
        iq20 x2;
        iq20 x3 = 0;
        RoundtripStage stage;
        uint32_t t_stagelocal;
        
        uint32_t t_base = 0;
        if(t >= t_total){
            t_stagelocal = t - t_total;
            p64 = p64_initial;
            x2 = 0;
            x3 = 0;
            stage = RoundtripStage::BackwardDeacc;
        } else if (t_base += t_acc; t < t_base) {
            // 阶段1: 加速
            t_stagelocal = t;
            p64 = p64_initial + calc_accdec_x1delta(t_stagelocal);
            x2 = calc_accdec_x2(t_stagelocal);
            stage = is_growup ? RoundtripStage::InitialAcc : RoundtripStage::BackwardDeacc;
            x3 = calc_accdec_x3();
        } else if (t_base += t_buff_outer_elapsed; t < t_base) {
            // 阶段2: 外缓冲
            t_stagelocal = t - t_acc;
            const uint32_t t_reversed = (t_acc + t_buff_outer_elapsed) - t;
            p64 = p64_entry_uniform - calc_uniform_x1delta(t_reversed);
            x2 = calc_uniform_x2();
            stage = is_growup ? RoundtripStage::ForwardEntryBuffering 
                            : RoundtripStage::BackwardExitBuffering;
            
        } else if (t_base += t_uniform_elapsed; t < t_base) {
            // 阶段3: 匀速
            t_stagelocal = t - (t_acc + t_buff_outer_elapsed);
            p64 = p64_entry_uniform + calc_uniform_x1delta(t_stagelocal);
            x2 = calc_uniform_x2();
            stage = is_growup ? RoundtripStage::ForwardSpin 
                            : RoundtripStage::BackwardSpin;
            
        } else if (t_base += t_buff_inner_elapsed; t < t_base) {
            // 阶段4: 内缓冲
            t_stagelocal = t - (t_acc + t_buff_outer_elapsed + t_uniform_elapsed);
            p64 = p64_exit_uniform + calc_uniform_x1delta(t_stagelocal);
            x2 = calc_uniform_x2();
            stage = is_growup ? RoundtripStage::ForwardExitBuffering 
                            : RoundtripStage::BackwardEntryBuffering;
        } else {
            // 阶段5: 减速（包含后半段的反向加速）
            t_stagelocal = t - (t_acc + t_buff_outer_elapsed + t_uniform_elapsed + t_buff_inner_elapsed);
            p64 = p64_exit_buffering + calc_uniform_x1delta(t_stagelocal) 
                - calc_accdec_x1delta(t_stagelocal);

            x2 = calc_uniform_x2() - calc_accdec_x2(t_stagelocal);
            x3 = -calc_accdec_x3();
            stage = is_growup ? RoundtripStage::ForwardDeacc 
                            : RoundtripStage::BackwardAcc;

        }
        

        if(not is_growup){
            x2 = -x2;
        }

        return RoundtripSamplePoint{
            .x1 = iiq32::from_bits(p64),
            .x2 = x2,
            .x3 = x3,
            .t_stagelocal = t_stagelocal,
            .stage = stage,
        };
    }
private:
    static constexpr int64_t _p64_add_revs(int64_t x, int32_t n_revs) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + n_revs) << 32) | frac;
    }

    static constexpr int64_t _ceil_p64(int64_t x) {
        const uint32_t frac = uint32_t(x & UINT32_MAX);
        const int32_t revs = int32_t(x >> 32);
        return int64_t(int64_t(revs + bool(frac)) << 32);
    }

    static constexpr int64_t _floor_p64(int64_t x) {
        static constexpr int64_t MASK = int64_t(UINT64_MAX & (~uint64_t(UINT32_MAX)));
        return x & MASK;
    }

    template<typename T>
    static constexpr T _conditional_neg(const bool cond, const T x){
        if(cond) return -x;
        return x;
    }

    static constexpr uint32_t _rcp_u32(const uint32_t x){
        return (0xffff'ffff / x) + 1;
    }

    template<typename T>
    static constexpr T _abs_diff(const T a, const T b){
        if(a > b) return a - b;
        return b - a;
    }

    template<typename T>
    static constexpr T _abs(const T x){
        if(x < 0) return -x;
        return x;
    }
};
}