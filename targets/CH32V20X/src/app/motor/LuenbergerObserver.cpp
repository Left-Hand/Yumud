#include "LuenbergerObserver.hpp"

namespace ymd::foc {

LuenbergerObserver::LuenbergerObserver(const iq_t _A11, const iq_t _A12, const iq_t _A21, const iq_t _A22,
                                       const iq_t _B1, const iq_t _B2,
                                       const iq_t _L11, const iq_t _L12, const iq_t _L21, const iq_t _L22)
    : A11(_A11), A12(_A12), A21(_A21), A22(_A22),
      B1(_B1), B2(_B2),
      L11(_L11), L12(_L12), L21(_L21), L22(_L22),
      theta_hat(0), omega_hat(0),
      Kp(1.0_r), Ld(0.1_r), Lq(0.1_r), R(0.1_r),
      IalphaError(0), IbetaError(0) {}

void LuenbergerObserver::init() {
    theta_hat = 0;
    omega_hat = 0;
}

void LuenbergerObserver::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta) {
    // 计算电流误差
    IalphaError = Ialpha - EstIalpha;
    IbetaError = Ibeta - EstIbeta;

    // 状态估计更新
    theta_hat += (A11 * theta_hat + A12 * omega_hat + B1 * Valpha + L11 * IalphaError + L12 * IbetaError);
    omega_hat += (A21 * theta_hat + A22 * omega_hat + B2 * Vbeta + L21 * IalphaError + L22 * IbetaError);

    // 估计电流
    EstIalpha = Kp * (Ld * omega_hat * sin(theta_hat) + Lq * omega_hat * cos(theta_hat)) + R * Ialpha;
    EstIbeta = Kp * (Ld * omega_hat * cos(theta_hat) - Lq * omega_hat * sin(theta_hat)) + R * Ibeta;
}

iq_t LuenbergerObserver::getTheta() const {
    return theta_hat;
}

iq_t LuenbergerObserver::getOmega() const {
    return omega_hat;
}

} // namespace ymd