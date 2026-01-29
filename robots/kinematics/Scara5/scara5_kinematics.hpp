#pragma once

#include "algebra/vectors/vec2.hpp"
#include "core/math/realmath.hpp"

namespace ymd::robots::kinematics{
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
    constexpr math::Vec2<T> forward(const Angular<T> left,const Angular<T> right){
        const auto l_pos = math::Vec2<T>(-cfg_.should_length / 2, 0);
        const auto r_pos = math::Vec2<T>(cfg_.should_length / 2, 0);

        const math::Vec2<T> l_elbow_pos = l_pos + math::Vec2<T>(cfg_.upperarm_length, 0).rotated(left);
        const math::Vec2<T> r_elbow_pos = r_pos + math::Vec2<T>(cfg_.upperarm_length, 0).rotated(right);

        const math::Vec2<T> lr_diff = r_elbow_pos - l_elbow_pos;
        const math::Vec2<T> lr_mid = (l_elbow_pos + r_elbow_pos) / 2;
        const T beak_len = sqrt(cfg_.forearm_length * cfg_.forearm_length - 
            (lr_diff.length_squared() / 4));

        return lr_mid + lr_diff.normalized().cw() * beak_len;
    }

constexpr std::tuple<Angular<T>, Angular<T>> inverse(const math::Vec2<T> & pos){
        const auto l_pos = math::Vec2<T>(-cfg_.should_length / 2, 0);
        const auto r_pos = math::Vec2<T>(cfg_.should_length / 2, 0);
        const math::Vec2<T> l_offs = pos - l_pos;
        
        const T l_squ = l_offs.length_squared();
        const Angular<T> l_theta = l_offs.angle() + 
            Angular<T>::from_radians(
                acos((math::square(cfg_.upperarm_length) + l_squ - math::square(cfg_.forearm_length)) 
                / (2 * cfg_.upperarm_length * sqrt(l_squ))));

        const math::Vec2<T> r_offs = pos - r_pos;
        const T r_squ = r_offs.length_squared();
        const Angular<T> r_theta = r_offs.angle() - 
            Angular<T>::from_radians(acos((math::square(cfg_.upperarm_length) + r_squ - math::square(cfg_.forearm_length)) 
                / (2 * cfg_.upperarm_length * sqrt(r_squ))));

        return {l_theta, r_theta};
    }
};

}