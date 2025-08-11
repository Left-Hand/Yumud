#include "HfiObserver.hpp"
#include "core/math/realmath.hpp"

namespace ymd::foc {

HfiObserver::HfiObserver(const q16 _f_inj, const q16 _Kf, const q16 _Ki)
    : 
        f_inj(_f_inj),
        Kf(_Kf),
        Ki(_Ki),
        theta_hat(0),
        omega_hat(0),
        Kp(1.0_r),
        Ld(0.1_r),
        Lq(0.1_r),
        R(0.1_r),
        IalphaError(0),
        IbetaError(0),
        integral_IalphaError(0),
        integral_IbetaError(0) {}

void HfiObserver::init() {
    theta_hat = 0;
    omega_hat = 0;
    integral_IalphaError = 0;
    integral_IbetaError = 0;
}

void HfiObserver::update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta) {
    // 计算电流误差
    IalphaError = Ialpha - EstIalpha;
    IbetaError = Ibeta - EstIbeta;

    // 积分电流误差
    integral_IalphaError += IalphaError;
    integral_IbetaError += IbetaError;

    // 状态估计更新

    auto [s_inj, c_inj] = sincos(f_inj);
    
    theta_hat += Kf * (IalphaError * s_inj + IbetaError * c_inj) + Ki * integral_IalphaError;
    omega_hat += Kf * (IalphaError * c_inj - IbetaError * s_inj) + Ki * integral_IbetaError;
    
    auto [s, c] = sincos(theta_hat);

    // 估计电流
    EstIalpha = Kp * (Ld * omega_hat * s + Lq * omega_hat * c) + R * Ialpha;
    EstIbeta =  Kp * (Ld * omega_hat * c - Lq * omega_hat * s) + R * Ibeta;
}

q16 HfiObserver::getTheta() const {
    return theta_hat;
}

q16 HfiObserver::getOmega() const {
    return omega_hat;
}

} // namespace ymd