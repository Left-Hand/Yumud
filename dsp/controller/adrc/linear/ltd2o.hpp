#pragma once

#include "dsp/controller/adrc/prelude.hpp"


namespace ymd::dsp::adrc{



//基于高志强改编的二阶线性微分跟踪器 假定加速度是慢时变信号
template<>
struct [[nodiscard]] LinearTrackingDifferentiator<iq16, 2>{
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
        //采样间隔时间
        uq32 dt;

        // r / fs
        uq32 r_by_fs;

        // r^2 / fs
        uq16 r2_by_fs;

        friend OutputStream & operator<<(OutputStream & os, const Coeffs & self){
            return os << os.field("dt")(self.dt) << os.splitter() 
                << os.field("r_by_fs")(self.r_by_fs) << os.splitter()
                << os.field("r2_by_fs")(self.r2_by_fs);
        }
    };

    struct [[nodiscard]] Config{
        // 采样频率
        uint32_t fs;

        // 快速因子r 通过r的频率的信号响应减半
        uint32_t r;

        constexpr Result<Coeffs, StringView> try_to_coeffs() const {
            auto & self = *this;

            if(fs >= 65536) [[unlikely]]
                return Err(StringView("fs >= 65536"));
            if(r >= 65536) [[unlikely]]
                return Err(StringView("r >= 65536"));
            if(r >= fs) [[unlikely]]
                return Err(StringView("r >= fs"));

            const auto dt = std::numeric_limits<uq32>::max() / self.fs;
            const auto r_by_fs = uq32::from_bits(static_cast<uint32_t>(
                uint64_t(r) * uint64_t(uint64_t(1) << 32) / fs));
            const auto r2_by_fs = uq16::from_bits(static_cast<uint32_t>(
                uint64_t(r)  * uint64_t(r) * uint64_t(uint64_t(1) << 16) / fs));

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

    using State = SecondOrderState<iq16>;
    constexpr State update(
        const State state, 
        const std::array<iq16, 2> ref
    ) const {

        const auto x1_now = state.x1;
        const auto x2_now = state.x2;

        // 临界二阶阻尼系统
        
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        const auto x1_now_q16 = math::fixed_downcast<16>(x1_now);

        [[maybe_unused]] const iq16 e1 = ref[0] - x1_now_q16;
        [[maybe_unused]] const iq16 e2 = ref[1] - x2_now;

        const auto next_x1 = x1_now + static_cast<iiq32>(extended_mul(x2_now, coeffs_.dt));
        const auto next_x2 = x2_now + math::fixed_downcast<16>(
            extended_mul(iq16(2 * e2), coeffs_.r_by_fs) 
            + extended_mul(iq16(e1), coeffs_.r2_by_fs));

        return SecondOrderState<iq16>{
            next_x1,
            next_x2
        };
    }

private:
    Coeffs coeffs_;
};


}