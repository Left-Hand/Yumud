#include "HfiObserver.hpp"

namespace ymd::foc {

HfiObserver::HfiObserver(const iq_t _f_inj, const iq_t _Kf, const iq_t _Ki)
    : f_inj(_f_inj),
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

void HfiObserver::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta) {
    // 计算电流误差
    IalphaError = Ialpha - EstIalpha;
    IbetaError = Ibeta - EstIbeta;

    // 积分电流误差
    integral_IalphaError += IalphaError;
    integral_IbetaError += IbetaError;

    // 状态估计更新
    theta_hat += Kf * (IalphaError * sin(f_inj) + IbetaError * cos(f_inj)) + Ki * integral_IalphaError;
    omega_hat += Kf * (IalphaError * cos(f_inj) - IbetaError * sin(f_inj)) + Ki * integral_IbetaError;

    // 估计电流
    EstIalpha = Kp * (Ld * omega_hat * sin(theta_hat) + Lq * omega_hat * cos(theta_hat)) + R * Ialpha;
    EstIbeta = Kp * (Ld * omega_hat * cos(theta_hat) - Lq * omega_hat * sin(theta_hat)) + R * Ibeta;
}

iq_t HfiObserver::getTheta() const {
    return theta_hat;
}

iq_t HfiObserver::getOmega() const {
    return omega_hat;
}

} // namespace ymd