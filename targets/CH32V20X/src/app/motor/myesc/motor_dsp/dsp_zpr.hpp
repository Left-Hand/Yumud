// https://zhuanlan.zhihu.com/p/699288152

// 具有相位补偿的pr控制器
#include <array>
#include <cmath>

namespace ymd::dsp{

struct ZprCoeffs{
    std::array<float, 3> b;
    std::array<float, 2> a;
};

struct ZprConfig{
    // float kp;
    // float kr;
    float w0;
    float wc;
    float dt;

    #if 0
    constexpr ZprCoeffs calc() const {
        float a0 = (4.0f / (dt*dt)) + (4.0f * wc / dt) + w0 * w0;
        float a1 = (-8.0f / (dt*dt)) + 2.0f * w0 * w0;
        float a2 = (4.0f / (dt*dt)) - (4.0f * wc / dt) + w0 * w0;
    
        // 计算分子系数
        float b0 = (4.0f * kp) / (dt*dt) + (4.0f * wc * (kp + kr) / dt) + kp * w0 * w0;
        float b1 = (-8.0f * kp) / (dt*dt) + 2.0f * kp * w0 * w0;
        float b2 = (4.0f * kp) / (dt*dt) - (4.0f * wc * (kp + kr) / dt) + kp * w0 * w0;

        b0 = b0 / a0;
        b1 = b1 / a0;
        b2 = b2 / a0;
        a1 = a1 / a0;
        a2 = a2 / a0;

        return ZprCoeffs{
            .b = {b0, b1, b2},
            .a = {a1, a2}
        };
    }
    #endif

    constexpr ZprCoeffs calc_with_phase_comp(float phi) const {
        
        float p0 = std::sin(w0 * dt) * std::cos(phi);
        float p1 = - std::sin(phi) * (1 - std::cos(w0 * dt));

        float wc_by_w0 = wc / w0;
        float q1 = wc_by_w0 * sin(w0 * dt);


        float a0 = 1 + q1;
        float a1 = - 2 * cos(w0 * dt);
        float a2 = 1 - q1;

        float b0 = wc_by_w0 * (p0 + p1);
        float b1 = wc_by_w0 * (2 * p1);
        float b2 = wc_by_w0 * (p1 - p0);

        b0 = b0 / a0;
        b1 = b1 / a0;
        b2 = b2 / a0;
        a1 = a1 / a0;
        a2 = a2 / a0;

        return ZprCoeffs{
            .b = {b0, b1, b2},
            .a = {a1, a2}
        };
    }

}

}