#pragma once

#include "core/math/real.hpp"
#include "algebra/vectors/vec2.hpp"
#include "core/utils/Option.hpp"

namespace ymd::rmst{


using Radian = Radian_t<iq16>;

using RadianPair = std::tuple<Radian, Radian>;




class LegKinematics{
    struct Config{
        iq16 rotor_length;
        iq16 hipbone_length;
        iq16 thigh_length;
        iq16 shank_length;
    };

    static constexpr Config default_cfg {
        .rotor_length = 0.09_r,
        .hipbone_length = 0.11_r,
        .thigh_length = 0.20_r,
        .shank_length = 0.244_r
    };

    void reset(){

    }

    void reconf(const Config & cfg){
        rotor_length_ = cfg.rotor_length;
        hipbone_length_ = cfg.hipbone_length;
        thigh_length_ = cfg.thigh_length;
        shank_length_ = cfg.shank_length;
    }

    Option<Vec2<iq16>> forward(const Radian l_rad, const Radian r_rad){
        return _forward(l_rad, r_rad);
    }

    Option<RadianPair> inverse(Vec2<iq16> const pos){
        return _inverse(pos);
    }
private:
    iq16 rotor_length_;
    iq16 hipbone_length_;
    iq16 thigh_length_;
    iq16 shank_length_;

    Option<Vec2<iq16>> _forward(const Radian l_rad, const Radian r_rad) const{
        return Some(Vec2<iq16>(0,0));
    }
    
    Option<RadianPair> _inverse(Vec2<iq16> const pos) const;
};
// }

// template<arithmetic T>
// class LegForwardKinematics:public LegKinamaticsBase<T>{
//     using Config = details::LegKinamaticsBase<T>::Config;
//     Config config;
// };

}