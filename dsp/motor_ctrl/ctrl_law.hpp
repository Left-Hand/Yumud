#pragma once

#include "core/math/iq/iqmath.hpp"


namespace ymd::dsp{
struct PdCtrlLaw final{
    q16 kp;
    q16 kd;

    constexpr q16 operator()(const q16 p_err, const q16 v_err) const {
        return kp * p_err + kd * v_err;
    } 
};

struct SqrtKpCtrlLaw final{
    q16 kp;
    q16 ks;

    constexpr q16 operator()(const q16 p_err) const {
        const bool is_neg = p_err < 0;
        const auto abs_p_err = ABS(p_err);
        const auto abs_ret = MIN(kp * abs_p_err, ks * sqrt(abs_p_err));
        return is_neg ? -abs_ret : abs_ret;
    } 
};

struct SqrtPdCtrlLaw final{
    q16 kp;
    q16 ks;
    q16 kd;

    constexpr q16 operator()(const q16 p_err, const q16 v_err) const {
        return SqrtKpCtrlLaw{.kp = kp, .ks = ks}(p_err) + kd * v_err;
    } 
};

}