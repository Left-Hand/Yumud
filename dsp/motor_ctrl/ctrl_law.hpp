#pragma once

#include "core/math/fixed/fxmath.hpp"


namespace ymd::dsp{
struct PdCtrlLaw final{
    iq16 kp;
    iq16 kd;

    constexpr iq16 operator()(const iq16 p_err, const iq16 v_err) const {
        return kp * p_err + kd * v_err;
    } 
};

struct SqrtKpCtrlLaw final{
    iq16 kp;
    iq16 ks;

    constexpr iq16 operator()(const iq16 p_err) const {
        const bool is_neg = p_err < 0;
        const auto abs_p_err = math::abs(p_err);
        const auto abs_ret = MIN(kp * abs_p_err, ks * math::sqrt(abs_p_err));
        return is_neg ? -abs_ret : abs_ret;
    } 
};

struct SqrtPdCtrlLaw final{
    iq16 kp;
    iq16 ks;
    iq16 kd;

    constexpr iq16 operator()(const iq16 p_err, const iq16 v_err) const {
        return SqrtKpCtrlLaw{.kp = kp, .ks = ks}(p_err) + kd * v_err;
    } 
};

}