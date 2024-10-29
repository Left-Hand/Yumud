#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"

template<typename T>
class Scara5Solver_t{
public:
    struct Config{
        const T should_length_meter;
        const T forearm_length_meter;
        const T upperarm_length_meter;
    };

protected:
    const Config & config;
public:
    constexpr Scara5Solver_t(const Config & _config): config(_config){;}
    Vector2_t<T> forward(const T left_rad,const T right_rad){
        auto l_pos = Vector2_t<T>(-config.should_length_meter / 2, 0);
        auto r_pos = Vector2_t<T>(config.should_length_meter / 2, 0);

        Vector2_t<T> l_elbow_pos = l_pos + Vector2_t<T>(config.upperarm_length_meter, 0).rotated(left_rad);
        Vector2_t<T> r_elbow_pos = r_pos + Vector2_t<T>(config.upperarm_length_meter, 0).rotated(right_rad);

        Vector2_t<T> lr_diff = r_elbow_pos - l_elbow_pos;
        Vector2_t<T> lr_mid = (l_elbow_pos + r_elbow_pos) / 2;
        T beak_len = sqrt(config.forearm_length_meter * config.forearm_length_meter - (lr_diff.length_squared() / 4));

        return lr_mid + lr_diff.normalized().cw() * beak_len;
    }

    std::tuple<T, T> invrese(const Vector2_t<T> & pos){
        auto l_pos = Vector2_t<T>(-config.should_length_meter / 2, 0);
        auto r_pos = Vector2_t<T>(config.should_length_meter / 2, 0);
        Vector2_t<T> l_offs = pos - l_pos;
        
        T l_squ = l_offs.length_squared();
        T l_theta = l_offs.angle() + 
            acos((config.upperarm_length_meter * config.upperarm_length_meter + l_squ - 
            config.forearm_length_meter * config.forearm_length_meter) / (2 * config.upperarm_length_meter * sqrt(l_squ)));

        Vector2_t<T> r_offs = pos - r_pos;
        T r_squ = r_offs.length_squared();
        T r_theta = r_offs.angle() - 
            acos((config.upperarm_length_meter * config.upperarm_length_meter + r_squ - 
            config.forearm_length_meter * config.forearm_length_meter) / (2 * config.upperarm_length_meter * sqrt(r_squ)));

        return {l_theta, r_theta};
    }
};