#pragma once

#include "sys/math/real.hpp"
#include "../SensorlessObserverIntf.hpp"

namespace ymd::foc{

class SmoObserver:public SensorlessObserverIntf{
public:
    SmoObserver(const SmoObserver & other) = delete;
    SmoObserver(SmoObserver && other) = default;

    // 构造函数
    SmoObserver(iq_t<16> _f_para, iq_t<16> _g_para, iq_t<16> _Kslide, iq_t<16> _Kslf);

    void reset();

    // 更新函数
    void update(iq_t<16> Valpha, iq_t<16> Vbeta, iq_t<16> Ialpha, iq_t<16> Ibeta);

    // 获取估计的转子角度
    iq_t<16> theta() const {return Theta;}

private:
    const iq_t<16> f_para;
    const iq_t<16> g_para;
    const iq_t<16> Kslide;
    const iq_t<16> Kslf;
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