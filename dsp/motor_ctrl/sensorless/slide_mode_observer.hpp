#pragma once

// https://blog.csdn.net/lijialin_bit/article/details/104263194
// https://geekdaxue.co/read/aiyanjiudexiaohutongxue@oo4p7l/pdapg4cn8io7ic6h

#include "core/math/real.hpp"
#include "core/math/realmath.hpp"

namespace ymd::dsp::motor_ctl{

class SlideModeObserver{
public:

    struct  Config{
        q16 f_para;
        q16 g_para;
        q16 kslide;   
        q16 kslf;   
    };

    constexpr SlideModeObserver(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reset(){
        e_alpha_ = 0;
        e_beta_ = 0;
        z_alpha_ = 0;
        z_beta_ = 0;
        est_i_alpha_ = 0;
        est_i_beta_ = 0;
        turns_ = 0;
    }



    // 更新函数
    constexpr void update(auto alphabeta_volt, auto alphabeta_curr){

        const auto [Valpha, Vbeta] = alphabeta_volt;
        const auto [Ialpha, Ibeta] = alphabeta_curr;

        // 滑模电流观测器
        est_i_alpha_ = (f_para_ * est_i_alpha_) + (g_para_ * (Valpha - e_alpha_ - z_alpha_));
        est_i_beta_ = (f_para_ * est_i_beta_) + (g_para_ * (Vbeta - e_beta_ - z_beta_));

        // 当前电流误差
        auto i_alpha_err = est_i_alpha_ - Ialpha;
        auto i_beta_err = est_i_beta_ - Ibeta;

        // 滑模控制计算器
        if (abs(i_alpha_err) < E0) {
            z_alpha_ = (Kslide_ * i_alpha_err * invE0);  // (Kslide_ * (i_alpha_err) / E0)
        } else if (i_alpha_err >= E0) {
            z_alpha_ = Kslide_;
        } else if (i_alpha_err <= -E0) {
            z_alpha_ = -Kslide_;
        }

        if (abs(i_beta_err) < E0) {
            z_beta_ = (Kslide_ * i_beta_err * invE0);  // (Kslide_ * (i_beta_err) / E0)
        } else if (i_beta_err >= E0) {
            z_beta_ = Kslide_;
        } else if (i_beta_err <= -E0) {
            z_beta_ = -Kslide_;
        }

        // 滑模控制滤波器 -> 反电动势计算器
        e_alpha_ = e_alpha_ + (Kslf_ * (z_alpha_ - e_alpha_));
        e_beta_ = e_beta_ + (Kslf_ * (z_beta_ - e_beta_));

        // 转子角度计算器 -> turns_ = atan(-e_alpha_, e_beta_)
        turns_ = frac(atan2pu(-e_alpha_, e_beta_));
    }

    constexpr void reconf(const Config & cfg){
        f_para_ = cfg.f_para;
        g_para_ = cfg.g_para;
        Kslide_ = cfg.kslide;
        Kslf_ = cfg.kslf;
    }


    // 获取估计的转子角度
    Angle<q16> angle() const {return Angle<q16>::from_turns(turns_);}

private:
    q16 f_para_ = 0;
    q16 g_para_ = 0;
    q16 Kslide_ = 0;
    q16 Kslf_ = 0;
public:
    q16 e_alpha_ = 0;
    q16 e_beta_ = 0;

    q16 z_alpha_ = 0;
    q16 z_beta_ = 0;
    
    q16 est_i_alpha_ = 0;
    q16 est_i_beta_ = 0;

    q16 turns_ = 0;

    // 滑模阈值
    static constexpr q16 E0 = q16(1.5);
    // 滑模阈值的倒数
    static constexpr q16 invE0 = q16(1/1.5);
};

} // namespace ymd