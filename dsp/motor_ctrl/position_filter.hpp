#pragma once

#include "tracking_differentiator.hpp"

namespace ymd::dsp{

struct PositionFilter{
    using Td = MotorTrackingDifferentiator;
    using TdConfig = typename Td::Config;

    struct Config{
        uint32_t fs;
        q8 r;
    };

    constexpr PositionFilter(const Config & cfg):
        td_(Td{TdConfig{
            .fs = cfg.fs,
            .r = cfg.r
        }}){
    }

    constexpr void reconf(const Config & cfg){
        td_.reconf(TdConfig{
            .fs = cfg.fs,
            .r = cfg.r
        });
    }

    constexpr void update(const q16 next_lap_position){
        if(unlikely(inited_ == false)){
            position_offset = map_lap_to_nearest(
                frac(next_lap_position - base_lap_position_));
            inited_ = true;
        }

        const auto delta_position = map_lap_postion_to_delta(
            lap_position_, next_lap_position);
        lap_position_ = next_lap_position;

        cont_position_ += delta_position;
        td_.update(cont_position_ + position_offset);
    }

    constexpr void set_base_lap_position(const q16 base_lap_position){
        base_lap_position_ = base_lap_position;
    }

    constexpr q16 lap_position() const{
        return lap_position_;
    }

    constexpr q20 position() const{
        return td_.get().position;
    }

    constexpr q16 cont_position() const {
        return cont_position_;
    }

    constexpr q20 speed() const {
        return td_.get().speed;
    }

    constexpr std::tuple<q16, q16> get_position_and_speed() const {
        return std::make_tuple(position(), speed());
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
    q16 lap_position_       = 0;
    q16 cont_position_      = 0;
    q16 base_lap_position_   = 0;
    q16 position_offset = 0;
    bool inited_ = false;
};

}