#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2.hpp"
#include "sys/utils/Option.hpp"

namespace ymd::rmst{


template<arithmetic T>
struct Radian_t{
    __inline constexpr explicit 
    Radian_t(const auto & rad):rad_(rad){}
    __inline constexpr explicit 
    operator T() const{return rad_;}
private:
    T rad_;
};

class LegKinematics{
    struct Config{
        real_t rotor_length;
        real_t hipbone_length;
        real_t thigh_length;
        real_t shank_length;
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

    Option<Vector2> forward(const real_t , const real_t knee_angle){
    }

    
private:
    real_t rotor_length_;
    real_t hipbone_length_;
    real_t thigh_length_;
    real_t shank_length_;
};
// }

// template<arithmetic T>
// class LegForwardKinematics:public LegKinamaticsBase<T>{
//     using Config = details::LegKinamaticsBase<T>::Config;
//     Config config;
// };

}