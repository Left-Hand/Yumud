#pragma once

#include "../prelude.hpp"


namespace ymd::dsp::adrc{


template<>
struct [[nodiscard]] NonlinearTrackingDifferentiator<iq16, 2>{
    using Fhan = FhanPrecomputed<iq16>;

    //预计算系数
    struct [[nodiscard]] Coeffs{
        //采样间隔时间
        uq32 dt;

        //FHAN算子
        Fhan fhan;

        //一阶导约束
        iq16 x2_limit;
    };

    //配置参数
    struct [[nodiscard]] Config{
        //采样频率
        uint32_t fs;

        //r参数(原始信号二阶导限幅)
        uq16 r;

        //滤波系数
        uq16 h;

        //原信号一阶导限幅
        iq16 x2_limit;

        constexpr Result<Coeffs, StringView> try_into_coeffs() const {
            const auto & self = *this;
            return Ok(Coeffs{
                .dt = uq32::from_rcp(fs), 
                .fhan = (Fhan(Fhan::Config{.r = self.r, .h = self.h})),
                .x2_limit = self.x2_limit
            });
        }
    };

    constexpr explicit NonlinearTrackingDifferentiator(const Coeffs & coeffs):
        coeffs_(coeffs)
        {;}


    // 纯函数 传递二阶状态向量和期望的状态 返回下一步的状态
    // x1' = x2
    // x2' = clamp(u, x2_limit)
    // u = fhan(e1, e2)

    constexpr SecondOrderState<iq16> update(
        const SecondOrderState<iq16> & state, 
        const std::array<iq16, 2> & ref 
    ) const {
        const iq16 x1_now = math::fixed_downcast<16>(state.x1);
        const iq16 x2_now = state.x2;

        const iq16 x1_ref = ref[0];
        const iq16 x2_ref = ref[1];
        
        const iq16 e1 = x1_ref - x1_now;
        const iq16 e2 = x2_ref - x2_now;
        const auto u = coeffs_.fhan({e1, e2});
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