#pragma once

// https://zhuanlan.zhihu.com/p/1937620403255215686


namespace ymd::dsp::motor_ctl{

template <typename T>
class SecondOrderLPF  {
    static constexpr T SQRT2_BY_2 = static_cast<T>(0.7071067811865475);

    T wc;   // 截止角频率 = 2*pi*fc
    T zeta;
    T Ts;
    T x1 = 0.0; // y
    T x2 = 0.0; // y_dot

    constexpr SecondOrderLPF(T fs, T fc, T zeta_ = SQRT2_BY_2) {
        Ts = 1.0 / fs;
        wc = 2.0 * M_PI * fc;
        zeta = zeta_;
    }

    constexpr std::array<T,2> deriv(T x1_, T x2_, T u) const {
        const T dx1 = x2_;
        const T dx2 = -wc*wc*x1_ - 2.0*zeta*wc*x2_ + wc*wc*u;
        return {dx1, dx2};
    }

    constexpr T process(T u) {
        auto k1 = deriv(x1, x2, u);
        auto k2 = deriv(x1 + Ts*0.5*k1[0], x2 + Ts*0.5*k1[1], u);
        auto k3 = deriv(x1 + Ts*0.5*k2[0], x2 + Ts*0.5*k2[1], u);
        auto k4 = deriv(x1 + Ts*k3[0],     x2 + Ts*k3[1],     u);

        x1 += Ts / 6.0 * (k1[0] + 2.0*k2[0] + 2.0*k3[0] + k4[0]);
        x2 += Ts / 6.0 * (k1[1] + 2.0*k2[1] + 2.0*k3[1] + k4[1]);

        return x1;
    }

    // 重置状态：几种模式可选
    // mode 0: steady-state by input (x1 = u0, x2 = 0)
    // mode 1: use measured output y0, set x1 = y0, set x2 so that x2' = 0  (减少加速度瞬态)
    // mode 2: set x1 = y0, x2 = dy0 (用户提供初始导数 dy0)
    constexpr void reset(T u0, T y0, int mode = 1, T dy0 = 0.0) {
        if (mode == 0) {
            x1 = u0;
            x2 = 0.0;
        } else if (mode == 1) {
            x1 = y0;
            // 避免除以零（zeta ~ 0），若zeta太小退回到x2=0
            if (std::abs(zeta) > 1e-8) {
                x2 = (wc / (2.0 * zeta)) * (u0 - x1); // 使得初始加速度为0
            } else {
                x2 = 0.0;
            }
        } else { // mode == 2
            x1 = y0;
            x2 = dy0;
        }
    }

    // 可选：平滑过渡（线性插值到目标状态，N步）warm-up trick 过程
    constexpr void smoothReset(T target_x1, T target_x2, int steps) {
        if (steps <= 1) {
            x1 = target_x1; x2 = target_x2; return;
        }
        T dx1 = (target_x1 - x1) / steps;
        T dx2 = (target_x2 - x2) / steps;
        for (int i = 0; i < steps; ++i) {
            x1 += dx1;
            x2 += dx2;
            // 也可以在这里调用 process(u_current) 若你想在过渡期间推进滤波器
        }
    }
};

}
