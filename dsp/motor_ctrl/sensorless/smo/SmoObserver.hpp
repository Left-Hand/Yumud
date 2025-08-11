#pragma once

// https://blog.csdn.net/lijialin_bit/article/details/104263194



#include "core/math/real.hpp"

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

    SmoObserver(const Config & cfg);

    void reconf(const Config & cfg){
        f_para_ = cfg.f_para;
        g_para_ = cfg.g_para;
        Kslide_ = cfg.kslide;
        Kslf_ = cfg.kslf;
    }

    void reset();

    // 更新函数
    void update(q16 Valpha, q16 Vbeta, q16 Ialpha, q16 Ibeta);

    // 获取估计的转子角度
    q16 theta() const {return Theta;}

private:
    q16 f_para_;
    q16 g_para_;
    q16 Kslide_;
    q16 Kslf_;
public:
    q16 Ealpha;
    q16 Ebeta;

    q16 Zalpha;
    q16 Zbeta;
    
    q16 EstIalpha;
    q16 EstIbeta;

    q16 Theta;

    // 滑模阈值
    static constexpr q16 E0 = q16(1.5);
    // 滑模阈值的倒数
    static constexpr q16 invE0 = q16(1/1.5);
};

} // namespace ymd