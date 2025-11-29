#pragma once

#include "core/math/iq/fixed_t.hpp"
#include "core/utils/Result.hpp"
#include "dsp/state_vector.hpp"
#include "fhan.hpp"

namespace ymd::dsp{



template<typename T, size_t Orders>
struct LinearTrackingDifferentiator;



template<>
struct LinearTrackingDifferentiator<iq16, 2>{
public:
    // https://zhuanlan.zhihu.com/p/510927934
    // 线性跟踪微分器是双极点滤波器在阻尼比等于1时的一种特例
    // 线性跟踪微分器更关注输入信号的处理，所以要保证滤波器的阶跃响应没有超调。
    // 但这也导致线性跟踪微分器相位滞后非常严重，不太适合用在反馈滤波环节上。

    // 线性跟踪微分器不适合用在反馈滤波上，指令滤波效果又比不上那些带零点的滤波器

    // 通过简单的整定，我们可以把中低频的模型问题和高频的未建模动态一并处理掉，
    // 一个系统总有一些高频的未建模动态，所以“加个线性跟踪微分器处理处理”总是一个不错的选择

    // 在实际使用中 当用于反馈时一般给线性跟踪微分器几百hz的截止频率

    // 1/(r^2 + fc^2) = 1/(r ^ 2) / sqrt(2)
    // =>  fc = r * sqrt(sqrt(2) - 1)
    // =>  fc = r * 0.6435

    using delta_type = uq16;
    struct [[nodiscard]] Coeffs{
        uq32 dt;
        uq16 r_by_fs;
        uq16 r2_by_fs;
    };

    struct [[nodiscard]] Config{
        // 采样频率
        uint32_t fs;
        // 快速因子r 通过r的频率的响应减半
        uint32_t r;

        constexpr Result<Coeffs, const char *> try_to_coeffs() const {
            auto & self = *this;
            const auto dt = uq32::from_bits(uint64_t(1ull << 32) / self.fs);
            const auto r_by_fs = uq16(r) / fs;
            const auto r2_by_fs = uq16(r * r) / fs;
            return Ok(Coeffs{
                dt,
                r_by_fs,
                r2_by_fs
            });
        }
    };


    constexpr explicit LinearTrackingDifferentiator(const Coeffs & coeffs):
        coeffs_(coeffs){
    }


    constexpr SecondOrderState update(const SecondOrderState state, const iq16 u) const {

        const auto x1 = state.x1;
        const auto x2 = state.x2;

        // 临界二阶阻尼系统
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        const auto x1_q16 = iq16::from_bits(x1.to_bits() >> 16);
        const auto delta_x1 = extended_mul(x2, coeffs_.dt);
        const auto delta_x2 = iq16::from_bits(
            fixed_t<48, int64_t>(extended_mul(iq16(-2 * x2), coeffs_.r_by_fs) 
            + extended_mul(iq16(u - x1_q16), coeffs_.r2_by_fs)).to_bits() >> 32);

        return SecondOrderState{
            x1 + delta_x1,
            x2 + delta_x2
        };
    }

private:
    Coeffs coeffs_;
};



struct NonlinearTrackingDifferentor{
    using fhan_type = FhanPrecomputed<iq16>;
    struct Coeffs{
        uq32 dt;
        fhan_type fhan;
        iq16 x2_limit;
    };

    struct Config{
        uint32_t fs;
        uq16 r;
        uq16 h;
        iq16 x2_limit;

        constexpr Result<Coeffs, const char *> try_to_coeffs() const {
            const auto & self = *this;
            return Ok(Coeffs{
                .dt = (std::numeric_limits<uq32>::max() / fs), 
                .fhan = (fhan_type(fhan_type::Config{.r = self.r, .h = self.h})),
                .x2_limit = self.x2_limit
            });
        }
    };

    constexpr explicit NonlinearTrackingDifferentor(const Coeffs & coeffs):
        coeffs_(coeffs)
        {;}
    

    constexpr SecondOrderState update(const SecondOrderState & state, const iq16 x1, const iq16 x2) const {
        const iq16 x1_now = iq16::from_bits(static_cast<int32_t>(state.x1.to_bits() >> 16));
        const iq16 x2_now = state.x2;
        
        const iq16 e1 = x1 - x1_now;
        const iq16 e2 = x2 - x2_now;
        const auto u = coeffs_.fhan(e1, e2);
        const auto next_x1 = state.x1 + extended_mul(state.x2, coeffs_.dt);
        const auto next_x2 = CLAMP2(state.x2 + u * coeffs_.dt, coeffs_.x2_limit);
        return {
            next_x1,
            next_x2
        };
    }

private:
    Coeffs coeffs_;
};

}