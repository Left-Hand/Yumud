#pragma once

#include "dsp/controller/adrc/prelude.hpp"

namespace ymd::dsp::adrc {

//基于高志强改编的二阶线性微分跟踪器 假定加速度是慢时变信号
template<>
struct [[nodiscard]] LinearTrackingDifferentiator<float, 2>{
public:
    // https://zhuanlan.zhihu.com/p/510927934
    // 线性跟踪微分器是双极点滤波器在阻尼比等于1时的一种特例
    // 线性跟踪微分器更关注输入信号的处理，所以要保证滤波器的阶跃响应没有超调。
    // 但这也导致线性跟踪微分器相位滞后非常严重，不太适合用在反馈滤波环节上。

    // 线性跟踪微分器不适合用在反馈滤波上，指令滤波效果又比不上那些带零点的滤波器

    // 通过简单的整定，我们可以把中低频的模型问题和高频的未建模动态一并处理掉，
    // 一个系统总有一些高频的未建模动态，所以"加个线性跟踪微分器处理处理"总是一个不错的选择

    // 在实际使用中 当用于反馈时一般给线性跟踪微分器几百hz的截止频率

    // 1/(r^2 + fc^2) = 1/(r ^ 2) / sqrt(2)
    // =>  fc = r * sqrt(sqrt(2) - 1)
    // =>  fc = r * 0.6435

    // 阶越时域表达式
    // \left(1-e^{-rx}-rxe^{-rx}\right)
    // \ g'\left(x\right)=r^{2}xe^{-rx}

    // 对于单位阶跃信号 一阶导最大绝对值为r/e

    struct [[nodiscard]] Precomputed{
        //采样间隔时间
        float dt;

        // r / fs
        float r_by_fs;

        // r^2 / fs
        float r2_by_fs;

        friend OutputStream & operator<<(OutputStream & os, const Precomputed & self){
            return os << os.field("dt")(self.dt) << os.splitter() 
                << os.field("r_by_fs")(self.r_by_fs) << os.splitter()
                << os.field("r2_by_fs")(self.r2_by_fs);
        }
    };

    struct [[nodiscard]] Config{
        // 采样频率
        float fs;

        // 快速因子r 通过r的频率的信号响应减半
        float r;

        constexpr Result<Precomputed, StringView> try_into_precomputed() const noexcept {
            auto & self = *this;

            if(fs <= 0.0) [[unlikely]]
                return Err(StringView("fs <= 0"));
            if(r <= 0.0) [[unlikely]]
                return Err(StringView("r <= 0"));
            if(r >= fs) [[unlikely]]
                return Err(StringView("r >= fs"));

            const auto dt = 1.0 / self.fs;
            const auto r_by_fs = self.r / self.fs;
            const auto r2_by_fs = (self.r * self.r) / self.fs;

            return Ok(Precomputed{
                dt,
                r_by_fs,
                r2_by_fs
            });
        }
    };

    constexpr explicit LinearTrackingDifferentiator(const Precomputed & coeffs):
        coeffs_(coeffs){
    }

    using State = SecondOrderState<float>;
    constexpr void iterate(
        State & state, 
        const std::array<float, 2> ref
    ) const noexcept {

        auto & x1_now = state.x1;
        auto & x2_now = state.x2;

        // 临界二阶阻尼系统
        
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        const float e1 = ref[0] - x1_now;
        const float e2 = ref[1] - x2_now;

        x1_now = x1_now + x2_now * coeffs_.dt;
        x2_now = x2_now + (2.0 * e2 * coeffs_.r_by_fs) + (e1 * coeffs_.r2_by_fs);
    }

private:
    Precomputed coeffs_;
};

}