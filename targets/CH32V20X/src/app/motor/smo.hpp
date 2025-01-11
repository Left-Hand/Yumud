#pragma once

#include "sys/math/real.hpp"  // 假设 iq_t 类型定义在这个头文件中

namespace ymd::foc{

class SmoPos {
public:
    SmoPos(const SmoPos & other) = delete;
    SmoPos(SmoPos && other) = default;

    // 构造函数
    SmoPos(iq_t _f_para, iq_t _g_para, iq_t _Kslide, iq_t _Kslf);

    // 初始化函数
    // void init(iq_t _g_para, iq_t _f_para, iq_t _Kslide, iq_t _Kslf);

    void reset();

    // 更新函数
    void update(iq_t Valpha, iq_t Vbeta, iq_t Ialpha, iq_t Ibeta);

    // 获取估计的转子角度
    iq_t getTheta() const;

private:
    // 参数: 电机相关的植物矩阵
    const iq_t f_para;
    const iq_t g_para;
    const iq_t Kslide;
    const iq_t Kslf;
public:
    iq_t Ealpha;
    iq_t Ebeta;

    iq_t Zalpha;
    iq_t Zbeta;
    
    iq_t EstIalpha;
    iq_t EstIbeta;

    iq_t IalphaError;
    iq_t IbetaError;
    iq_t Theta;

    // 滑模阈值
    static constexpr iq_t E0 = iq_t(1.5);
    // 滑模阈值的倒数
    static constexpr iq_t invE0 = 1 / E0;
};

} // namespace ymd