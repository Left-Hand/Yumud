#pragma once

#include "core/math/real.hpp"

namespace ymd::foc {

class HfiObserver {
public:
    struct Config{

    };

    HfiObserver(const q16 _f_inj, const q16 _Kf, const q16 _Ki);

    // 初始化函数
    void init();

    // 更新函数
    void update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta);

    // 获取估计的转子角度
    q16 getTheta() const;

    // 获取估计的转子速度
    q16 getOmega() const;

private:
    // 高频注入频率
    const q16 f_inj;

    // 观测器增益
    const q16 Kf;
    const q16 Ki;

    // 状态估计
    q16 theta_hat;
    q16 omega_hat;

    // 电机参数
    const q16 Kp;  // 电机常数
    const q16 Ld;  // 直轴电感
    const q16 Lq;  // 交轴电感
    const q16 R;   // 电机电阻

    // 电流误差
    q16 IalphaError;
    q16 IbetaError;

    q16 EstIalpha;
    q16 EstIbeta;

    // 积分项
    q16 integral_IalphaError;
    q16 integral_IbetaError;
};

} // namespace ymd