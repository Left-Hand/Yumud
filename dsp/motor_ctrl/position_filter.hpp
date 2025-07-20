#pragma once

#include "tracking_differentiator.hpp"
#include "position_corrector.hpp"

namespace ymd::dsp{

struct PositionFilter{
    using Td = MotorTrackingDifferentiator;
    using TdConfig = typename Td::Config;

    struct Config{
        q8 r;
        uint32_t fs;
        Option<PositionCorrector &> corrector = None;
    };

    constexpr PositionFilter(const Config & cfg):
        td_(Td{TdConfig{
            .r = cfg.r,
            .fs = cfg.fs
        }}),
        may_corrector_(cfg.corrector){
    }

    constexpr void reconf(const Config & cfg){
        td_.reconf(TdConfig{
            .r = cfg.r,
            .fs = cfg.fs
        });
    }

    constexpr void update(const q16 next_raw_lap_position){
        if(unlikely(inited_ == false)){
            position_offset = map_lap_to_nearest(
                frac(next_raw_lap_position - base_lap_position_));
            inited_ = true;
        }

        const auto corrected_lap_position = [this](const q16 raw_lap_position){
            if(may_corrector_.is_some())
                return may_corrector_.unwrap().correct_raw_position(raw_lap_position);
            else
                return raw_lap_position;
        }(next_raw_lap_position);

        const auto delta_position = map_lap_postion_to_delta(
            lap_position_, corrected_lap_position);
        lap_position_ = corrected_lap_position;

        cont_position_ += delta_position;
        td_.update(cont_position_ + position_offset);
    }

    constexpr void set_base_lap_position(const q16 base_lap_position){
        base_lap_position_ = base_lap_position;
    }

    constexpr q16 lap_position() const{
        return lap_position_;
    }

    constexpr q16 position() const{
        return td_.get().position;
    }

    constexpr q16 cont_position() const {
        return cont_position_;
    }

    constexpr q16 speed() const {
        return td_.get().speed;
    }

private:
    static constexpr q16 map_lap_to_nearest(const q16 x){
        if(x > 0.5_q16) return x - 1;
        return x;
    }
    static constexpr q16 map_lap_postion_to_delta(const q16 last_lap_position, const q16 lap_position){
        const auto delta = lap_position - last_lap_position;
        if(delta > 0.5_q16) return delta - 1;
        else if(delta < -0.5_q16) return delta + 1;
        return delta;
    }

    Td td_;
    Option<PositionCorrector &> may_corrector_;
    q16 lap_position_       = 0;
    q16 cont_position_      = 0;
    q16 base_lap_position_   = 0;
    q16 position_offset = 0;
    bool inited_ = false;
};

}