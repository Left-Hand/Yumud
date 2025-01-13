#pragma once

#include "sys/math/real.hpp"  // 假设 iq_t 类型定义在这个头文件中

namespace ymd::foc {

class LuenbergerObserver {
public:
    LuenbergerObserver(const iq_t _A11, const iq_t _A12, const iq_t _A21, const iq_t _A22,
                       const iq_t _B1, const iq_t _B2,
                       const iq_t _L11, const iq_t _L12, const iq_t _L21, const iq_t _L22);

    // 初始化函数
    void init();

    // 更新函数
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);

    // 获取估计的转子角度
    iq_t getTheta() const;

    // 获取估计的转子速度
    iq_t getOmega() const;

private:
    // 系统矩阵 A
    const iq_t A11;
    const iq_t A12;
    const iq_t A21;
    const iq_t A22;

    // 输入矩阵 B
    const iq_t B1;
    const iq_t B2;

    // 观测器增益矩阵 L
    const iq_t L11;
    const iq_t L12;
    const iq_t L21;
    const iq_t L22;

    // 状态估计
    iq_t theta_hat;
    iq_t omega_hat;

    iq_t EstIalpha;
    iq_t EstIbeta;
    
    // 电机参数
    const iq_t Kp;  // 电机常数
    const iq_t Ld;  // 直轴电感
    const iq_t Lq;  // 交轴电感
    const iq_t R;   // 电机电阻

    // 电流误差
    iq_t IalphaError;
    iq_t IbetaError;
};

} // namespace ymd