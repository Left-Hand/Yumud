#include "SmoObserver.hpp"
#include "core/math/realmath.hpp"

namespace ymd::foc{

// 构造函数
SmoObserver::SmoObserver(const Config & cfg){
    reconf(cfg);
    reset();
}

void SmoObserver::reset(){
    Ealpha = 0;
    Zalpha = 0;
    EstIalpha = 0;
    Ebeta = 0;
    Zbeta = 0;
    EstIbeta = 0;
    Theta = 0;
}



// 更新函数
void SmoObserver::update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta) {

    // 滑模电流观测器
    EstIalpha = (f_para_ * EstIalpha) + (g_para_ * (Valpha - Ealpha - Zalpha));
    EstIbeta = (f_para_ * EstIbeta) + (g_para_ * (Vbeta - Ebeta - Zbeta));

    // 当前电流误差
    auto IalphaError = EstIalpha - Ialpha;
    auto IbetaError = EstIbeta - Ibeta;

    // 滑模控制计算器
    if (abs(IalphaError) < E0) {
        Zalpha = (Kslide_ * IalphaError * invE0);  // (Kslide_ * (IalphaError) / E0)
    } else if (IalphaError >= E0) {
        Zalpha = Kslide_;
    } else if (IalphaError <= -E0) {
        Zalpha = -Kslide_;
    }

    if (abs(IbetaError) < E0) {
        Zbeta = (Kslide_ * IbetaError * invE0);  // (Kslide_ * (IbetaError) / E0)
    } else if (IbetaError >= E0) {
        Zbeta = Kslide_;
    } else if (IbetaError <= -E0) {
        Zbeta = -Kslide_;
    }

    // 滑模控制滤波器 -> 反电动势计算器
    Ealpha = Ealpha + (Kslf_ * (Zalpha - Ealpha));
    Ebeta = Ebeta + (Kslf_ * (Zbeta - Ebeta));

    // 转子角度计算器 -> Theta = atan(-Ealpha, Ebeta)
    Theta = atan2(-Ealpha, Ebeta);
}


} // namespace ymd