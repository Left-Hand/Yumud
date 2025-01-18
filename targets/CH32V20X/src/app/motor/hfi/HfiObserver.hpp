#pragma once

#include "sys/math/real.hpp"  // 假设 iq_t 类型定义在这个头文件中

namespace ymd::foc {

class HfiObserver {
public:
    HfiObserver(const iq_t _f_inj, const iq_t _Kf, const iq_t _Ki);

    // 初始化函数
    void init();

    // 更新函数
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);

    // 获取估计的转子角度
    iq_t getTheta() const;

    // 获取估计的转子速度
    iq_t getOmega() const;

private:
    // 高频注入频率
    const iq_t f_inj;

    // 观测器增益
    const iq_t Kf;
    const iq_t Ki;

    // 状态估计
    iq_t theta_hat;
    iq_t omega_hat;

    // 电机参数
    const iq_t Kp;  // 电机常数
    const iq_t Ld;  // 直轴电感
    const iq_t Lq;  // 交轴电感
    const iq_t R;   // 电机电阻

    // 电流误差
    iq_t IalphaError;
    iq_t IbetaError;

    iq_t EstIalpha;
    iq_t EstIbeta;

    // 积分项
    iq_t integral_IalphaError;
    iq_t integral_IbetaError;
};

} // namespace ymd