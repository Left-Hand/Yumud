#include "smo.hpp"

namespace ymd::foc{

// 构造函数
SmoPos::SmoPos(iq_t _f_para, iq_t _g_para,  iq_t _Kslide, iq_t _Kslf)
:   f_para(_f_para),
   g_para(_g_para),
   Kslide(_Kslide),
   Kslf(_Kslf)
    {

    reset();
}

void SmoPos::reset(){
    Ealpha = 0;
    Zalpha = 0;
    EstIalpha = 0;
    Ebeta = 0;
    Zbeta = 0;
    EstIbeta = 0;
    IalphaError = 0;
    IbetaError = 0;
    Theta = 0;
}



// 更新函数
void SmoPos::update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta) {

    // 滑模电流观测器
    EstIalpha = (f_para * EstIalpha) + (g_para * (Valpha - Ealpha - Zalpha));
    EstIbeta = (f_para * EstIbeta) + (g_para * (Vbeta - Ebeta - Zbeta));

    // 当前电流误差
    IalphaError = EstIalpha - Ialpha;
    IbetaError = EstIbeta - Ibeta;

    // 滑模控制计算器
    if (abs(IalphaError) < E0) {
        Zalpha = (Kslide * IalphaError * invE0);  // (Kslide * (IalphaError) / E0)
    } else if (IalphaError >= E0) {
        Zalpha = Kslide;
    } else if (IalphaError <= -E0) {
        Zalpha = -Kslide;
    }

    if (abs(IbetaError) < E0) {
        Zbeta = (Kslide * IbetaError * invE0);  // (Kslide * (IbetaError) / E0)
    } else if (IbetaError >= E0) {
        Zbeta = Kslide;
    } else if (IbetaError <= -E0) {
        Zbeta = -Kslide;
    }

    // 滑模控制滤波器 -> 反电动势计算器
    Ealpha = Ealpha + (Kslf * (Zalpha - Ealpha));
    Ebeta = Ebeta + (Kslf * (Zbeta - Ebeta));

    // 转子角度计算器 -> Theta = atan(-Ealpha, Ebeta)
    Theta = atan2(-Ealpha, Ebeta);
}

// 获取估计的转子角度
iq_t SmoPos::getTheta() const {
    return Theta;
}

} // namespace ymd