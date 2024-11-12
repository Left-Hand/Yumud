#pragma once

#include "types/vector2/vector2_t.hpp"

namespace yumud{
    
template<typename T>
class CrossSolver_t{
public:
    struct Config{
        const T xoffs_length_meter;
        const T forearm_length_meter;
        const T upperarm_length_meter;
    };
protected:
    const Config & config_;

    static T cosine_law(const T a, const T b, const T c){
        return acos((a*a + b*b - c*c) / (2*a*b));  
    }
    
    static T square(const T x){
        return x*x;
    }
public:
    constexpr CrossSolver_t(const Config & config):
        config_(config){;}


    constexpr CrossSolver_t(const CrossSolver_t<T> & other) = delete;
    constexpr CrossSolver_t(CrossSolver_t<T> && other) = delete;


    T forward(const T rad){
        auto base_pos = Vector2_t<T>{-config_.xoffs_length_meter, 0};
        auto jpos = base_pos + Vector2_t<T>{config_.upperarm_length_meter, 0}.rotated(rad);

        auto delta_h = sqrt(square(config_.forearm_length_meter) - square(jpos.x));
        return jpos.y + delta_h;
    }

    T inverse(const T height){
        auto bevel = Vector2_t<T>{config_.xoffs_length_meter, height};
        auto bevel_length = bevel.length();

        auto alpha = atan2(bevel.y, bevel.x);
        
        auto beta = cosine_law(
            bevel_length, 
            config_.upperarm_length_meter, 
            config_.forearm_length_meter
        );

        return alpha - beta;
    }
};


}