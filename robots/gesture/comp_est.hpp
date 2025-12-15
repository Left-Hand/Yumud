#pragma once

#include "algebra/vectors/vec3.hpp"
#include "dsp/filter/homebrew/complementary_filter.hpp"

namespace ymd::robots{
struct ComplementaryGestureEstimator{
    struct Config{
        uint32_t fs;
    };

    constexpr explicit ComplementaryGestureEstimator(const Config & cfg):
        delta_time_(1_r / cfg.fs),
        comp_filter_(make_comp_filter_config(cfg.fs)){;}

    constexpr void process(const Vec3<iq24> & x3,const Vec3<iq24> & x2){

        const auto len_x3 = x3.length();
        const auto norm_x3 = x3 / len_x3;
        const auto axis_theta_raw = math::atan2(norm_x3.x, norm_x3.y) + real_t(PI/2);
        const auto axis_omega_raw = x2.z;

        if(is_inited_ == false){
            theta_ = axis_theta_raw;
            omega_ = axis_omega_raw;
            is_inited_ = true;
            return;
        }

        const auto alpha_sqrt = (len_x3 - 9.8_r) * 0.8_r;

        const auto alpha = MAX(1 - math::square(alpha_sqrt), 0) * 0.04_r;

        theta_ += (axis_theta_raw - theta_) * alpha + (axis_omega_raw * delta_time_) * (1 - alpha);
        omega_ = axis_omega_raw;
    }

    auto theta_and_omega() const {
        return std::make_pair(theta_, omega_);
    }

private:
    using CompFilter = dsp::ComplementaryFilter<iq20>;
    using CompFilterConfig = typename CompFilter::Config;

    real_t delta_time_;
    CompFilter comp_filter_;
    real_t theta_ = 0;
    real_t omega_ = 0;
    bool is_inited_ = false;

    static constexpr CompFilterConfig
    make_comp_filter_config(const uint32_t fs){
        return {
            .kq = 0.90_r,
            .ko = 0.25_r,
            .fs = fs
        };
    }
};

}