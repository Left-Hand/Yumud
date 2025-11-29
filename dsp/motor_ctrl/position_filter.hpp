#pragma once

#include "dsp/controller/adrc/ltd2o.hpp"
#include "primitive/arithmetic/angle.hpp"

namespace ymd::dsp{


#if 1

class SecondOrderTrackingDifferentiator{
public:
    struct Config{
        uint32_t fs;
        iq8 r;
    };

    struct State{
        iq21 position;
        iq22 speed;

        constexpr void reset(){
            position = 0;
            speed = 0;
        }
    };


    constexpr explicit SecondOrderTrackingDifferentiator(const Config & cfg){
        reconf(cfg);
        reset();
    }

    constexpr void reconf(const Config & cfg){
        r_ = cfg.r;
        dt_ = 1_iq24 / cfg.fs;
    }

    constexpr void reset(){
        state_.reset();
    }

    constexpr void update(const iq16 u){
        const iq8 r = r_;
        const iq8 r_2 = r * r;

        const auto x1 = state_.position;
        const auto x2 = state_.speed;

        state_.position += dt_ * x2; 

        //临界二阶阻尼系统
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        state_.speed += dt_ * (iq16(- 2 * x2) * r + (iq16(u - x1) * r_2));
    }

    constexpr const State & state() const {return state_;}
private:
    iq8 r_ = 0;
    iq24 dt_ = 0;

    State state_;
};
#endif

struct PositionFilter{
    using Td = SecondOrderTrackingDifferentiator;
    using TdConfig = typename Td::Config;

    struct Config{
        uint32_t fs;
        iq8 r;
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

    constexpr void update(const Angle<iq16> next_lap_angle){
        const iq16 next_lap_turns = next_lap_angle.to_turns();
        if(inited_ == false) [[unlikely]] {
            angle_offset_ = map_lap_turns_to_nearest(
                frac(next_lap_turns - base_lap_turns_));
            inited_ = true;
        }

        const auto delta_angle = turns_diff(
            lap_turns_, next_lap_turns);
        lap_turns_ = next_lap_turns;

        cont_turns_ += delta_angle;
        td_.update(cont_turns_ + angle_offset_);
    }

    constexpr void set_base_lap_angle(const Angle<iq16> base_lap_angle){
        base_lap_turns_ = base_lap_angle.to_turns();
    }

    constexpr Angle<iq16> lap_angle() const{
        return Angle<iq16>::from_turns(lap_turns_);
    }

    constexpr Angle<iq20> accumulated_angle() const{
        return Angle<iq20>::from_turns(td_.state().position);
    }

    constexpr iq20 speed() const {
        return td_.state().speed;
    }

    constexpr void reset(){
        //TODO
    }

private:
    static constexpr iq16 map_lap_turns_to_nearest(const iq16 x){
        if(x > 0.5_iq16) return x - 1;
        return x;
    }
    static constexpr iq16 turns_diff(const iq16 last_lap_turns, const iq16 lap_turns){
        const auto delta = lap_turns - last_lap_turns;
        if(delta > 0.5_iq16) return delta - 1;
        else if(delta < -0.5_iq16) return delta + 1;
        return delta;
    }

    Td td_;
    iq16 lap_turns_       = 0;
    iq16 cont_turns_      = 0;
    iq16 base_lap_turns_   = 0;
    iq16 angle_offset_ = 0;
    bool inited_ = false;
};

}