#pragma once

#include "algebra/vectors/vec2.hpp"
#include "core/math/realmath.hpp"

namespace ymd::robots final{
    
template<typename T>
class CrossKinematics{
public:
    // -----*--------
    // ------\-------
    // -------\------
    // --------\-----
    // ---------*----
    // --------/-----
    // -------/------
    // ------/-------
    // -----/--------
    // ----/---------
    // ---*----------

    //实现一个图示的机械结构 活动关节在底部 连接有大臂和小臂 顶部的铰链可以上下活动


    struct Config{
        //X轴偏移(m)
        const T xoffs_length;

        //主臂长度(m)
        const T upperarm_length;
        //小臂长度(m)
        const T forearm_length;
    };
protected:
    const Config & config_;

    static constexpr T cosine_law(const T a, const T b, const T c){
        return acos((a*a + b*b - c*c) / (2*a*b));  
    }
    
public:
    constexpr CrossKinematics(const Config & config):
        config_(config){;}


    constexpr CrossKinematics(const CrossKinematics<T> & other) = delete;
    constexpr CrossKinematics(CrossKinematics<T> && other) = delete;


    constexpr T forward(const Angular<T> angle) const {
        auto base_pos = Vec2<T>{-config_.xoffs_length, 0};
        auto jpos = base_pos + Vec2<T>{config_.upperarm_length, 0}.rotated(angle);

        auto delta_h = sqrt(square(config_.forearm_length) - square(jpos.x));
        return jpos.y + delta_h;
    }

    T inverse(const T height) const {
        auto bevel = Vec2<T>{config_.xoffs_length, height};
        auto bevel_length = bevel.length();

        auto alpha = atan2(bevel.y, bevel.x);
        
        auto beta = cosine_law(
            bevel_length, 
            config_.upperarm_length, 
            config_.forearm_length
        );

        return alpha - beta;
    }
};


}