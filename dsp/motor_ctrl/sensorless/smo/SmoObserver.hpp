#pragma once

// https://blog.csdn.net/lijialin_bit/article/details/104263194

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::foc{

class SmoObserver{
public:
    SmoObserver(const SmoObserver & other) = delete;
    SmoObserver(SmoObserver && other) = default;

    struct  Config{
        q16 f_para;
        q16 g_para;
        q16 kslide;   
        q16 kslf;   
    };

    constexpr SmoObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reset(){
        Ealpha = 0;
        Zalpha = 0;
        EstIalpha = 0;
        Ebeta = 0;
        Zbeta = 0;
        EstIbeta = 0;
        Theta = 0;
    }



    // 更新函数
    constexpr void update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta) {

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

    constexpr void reconf(const Config & cfg){
        f_para_ = cfg.f_para;
        g_para_ = cfg.g_para;
        Kslide_ = cfg.kslide;
        Kslf_ = cfg.kslf;
    }


    // 获取估计的转子角度
    q16 theta() const {return Theta;}

private:
    q16 f_para_ = 0;
    q16 g_para_ = 0;
    q16 Kslide_ = 0;
    q16 Kslf_ = 0;
public:
    q16 Ealpha = 0;
    q16 Ebeta = 0;

    q16 Zalpha = 0;
    q16 Zbeta = 0;
    
    q16 EstIalpha = 0;
    q16 EstIbeta = 0;

    q16 Theta = 0;

    // 滑模阈值
    static constexpr q16 E0 = q16(1.5);
    // 滑模阈值的倒数
    static constexpr q16 invE0 = q16(1/1.5);
};

} // namespace ymd