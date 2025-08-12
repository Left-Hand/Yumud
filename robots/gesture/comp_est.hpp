#pragma once

#include "types/vectors/vector3.hpp"
#include "dsp/filter/homebrew/ComplementaryFilter.hpp"

namespace ymd::robots{
struct ComplementaryGestureEstimator{
    struct Config{
        uint32_t fs;
    };

    constexpr explicit ComplementaryGestureEstimator(const Config & cfg):
        delta_time_(1_r / cfg.fs),
        comp_filter_(make_comp_filter_config(cfg.fs)){;}

    constexpr void process(const Vector3<q24> & acc,const Vector3<q24> & gyr){

        const auto len_acc = acc.length();
        const auto norm_acc = acc / len_acc;
        const auto axis_theta_raw = atan2(norm_acc.x, norm_acc.y) + real_t(PI/2);
        const auto axis_omega_raw = gyr.z;

        if(is_inited_ == false){
            theta_ = axis_theta_raw;
            omega_ = axis_omega_raw;
            is_inited_ = true;
            return;
        }

        // const auto base_roll = comp_filter(axis_theta_raw, axis_omega_raw);
        // DEBUG_PRINTLN_IDLE(
        //     // norm_acc.x, norm_acc.y,
        //     // axis_theta_raw,
        //     // axis_omega_raw,
        //     // base_roll,
        //     // pos_filter_.position(),
        //     // pos_filter_.lap_position(),
        //     // pos_filter_.speed(),
        //     ma730_.read_lap_position().examine(),
        //     meas_elecrad_
        //     // exe_us_
        //     // // leso_.get_disturbance(),
        //     // meas_elecrad_
        // );

        const auto alpha_sqrt = (len_acc - 9.8_r) * 0.8_r;

        const auto alpha = MAX(1 - square(alpha_sqrt), 0) * 0.04_r;

        theta_ += (axis_theta_raw - theta_) * alpha + (axis_omega_raw * delta_time_) * (1 - alpha);
        omega_ = axis_omega_raw;
    }

    auto theta_and_omega() const {
        return std::make_pair(theta_, omega_);
    }

private:
    using CompFilter = dsp::ComplementaryFilter<q20>;
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