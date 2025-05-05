#pragma once

// https://blog.csdn.net/lijialin_bit/article/details/104263194



#include "core/math/real.hpp"

namespace ymd::foc{

class SmoObserver{
public:
    SmoObserver(const SmoObserver & other) = delete;
    SmoObserver(SmoObserver && other) = default;

    struct  Config{
        iq_t<16> f_para;
        iq_t<16> g_para;
        iq_t<16> kslide;   
        iq_t<16> kslf;   
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
    void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta);

    // 获取估计的转子角度
    iq_t<16> theta() const {return Theta;}

private:
    iq_t<16> f_para_;
    iq_t<16> g_para_;
    iq_t<16> Kslide_;
    iq_t<16> Kslf_;
public:
    iq_t<16> Ealpha;
    iq_t<16> Ebeta;

    iq_t<16> Zalpha;
    iq_t<16> Zbeta;
    
    iq_t<16> EstIalpha;
    iq_t<16> EstIbeta;

    iq_t<16> Theta;

    // 滑模阈值
    static constexpr iq_t<16> E0 = iq_t<16>(1.5);
    // 滑模阈值的倒数
    static constexpr iq_t<16> invE0 = iq_t<16>(1/1.5);
};

} // namespace ymd