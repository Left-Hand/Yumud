#pragma once

#include "algebra/vectors/vec2.hpp"
#include "core/math/realmath.hpp"

namespace ymd{
template<typename T>
class Scara5Kinematics{
public:
    struct Config{
        const T should_length;
        const T upperarm_length;
        const T forearm_length;
    };

protected:
    const Config & cfg_;
public:
    constexpr explicit Scara5Kinematics(const Config & cfg): cfg_(cfg){;}
    constexpr Vec2<T> forward(const T left_rad,const T right_rad){
        const auto l_pos = Vec2<T>(-cfg_.should_length / 2, 0);
        const auto r_pos = Vec2<T>(cfg_.should_length / 2, 0);

        const Vec2<T> l_elbow_pos = l_pos + Vec2<T>(cfg_.upperarm_length, 0).rotated(left_rad);
        const Vec2<T> r_elbow_pos = r_pos + Vec2<T>(cfg_.upperarm_length, 0).rotated(right_rad);

        const Vec2<T> lr_diff = r_elbow_pos - l_elbow_pos;
        const Vec2<T> lr_mid = (l_elbow_pos + r_elbow_pos) / 2;
        const T beak_len = sqrt(cfg_.forearm_length * cfg_.forearm_length - 
            (lr_diff.length_squared() / 4));

        return lr_mid + lr_diff.normalized().cw() * beak_len;
    }

    constexpr std::tuple<T, T> inverse(const Vec2<T> & pos){
        const auto l_pos = Vec2<T>(-cfg_.should_length / 2, 0);
        const auto r_pos = Vec2<T>(cfg_.should_length / 2, 0);
        const Vec2<T> l_offs = pos - l_pos;
        
        const T l_squ = l_offs.length_squared();
        const T l_theta = l_offs.angle() + 
            acos((cfg_.upperarm_length * cfg_.upperarm_length + l_squ - 
            cfg_.forearm_length * cfg_.forearm_length) / (2 * cfg_.upperarm_length * sqrt(l_squ)));

        const Vec2<T> r_offs = pos - r_pos;
        const T r_squ = r_offs.length_squared();
        const T r_theta = r_offs.angle() - 
            acos((cfg_.upperarm_length * cfg_.upperarm_length + r_squ - 
            cfg_.forearm_length * cfg_.forearm_length) / (2 * cfg_.upperarm_length * sqrt(r_squ)));

        return {l_theta, r_theta};
    }
};

}