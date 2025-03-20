#pragma once

#include "core/math/real.hpp"  // 假设 iq_t<16> 类型定义在这个头文件中

namespace ymd::foc {

class HfiObserver {
public:
    HfiObserver(const iq_t<16> _f_inj, const iq_t<16> _Kf, const iq_t<16> _Ki);

    // 初始化函数
    void init();

    // 更新函数
    void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta);

    // 获取估计的转子角度
    iq_t<16> getTheta() const;

    // 获取估计的转子速度
    iq_t<16> getOmega() const;

private:
    // 高频注入频率
    const iq_t<16> f_inj;

    // 观测器增益
    const iq_t<16> Kf;
    const iq_t<16> Ki;

    // 状态估计
    iq_t<16> theta_hat;
    iq_t<16> omega_hat;

    // 电机参数
    const iq_t<16> Kp;  // 电机常数
    const iq_t<16> Ld;  // 直轴电感
    const iq_t<16> Lq;  // 交轴电感
    const iq_t<16> R;   // 电机电阻

    // 电流误差
    iq_t<16> IalphaError;
    iq_t<16> IbetaError;

    iq_t<16> EstIalpha;
    iq_t<16> EstIbeta;

    // 积分项
    iq_t<16> integral_IalphaError;
    iq_t<16> integral_IbetaError;
};

} // namespace ymd