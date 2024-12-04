#pragma once

#include "types/ray2d/Ray2D_t.hpp"

namespace ymd{

template<arithmetic T>
struct PoseVelocity2D_t{
    Vector2_t<T> velocity;
    T spinrate;
};


template<arithmetic T>
class Mecanum4Solver_t{
public:
    using T4 = std::tuple<T, T, T, T>;
    
    struct Config{
        const T chassis_width_meter;
        const T chassis_height_meter;
    };

protected:
    const Config & config;

    Vector2_t<T> get_velocity_from_wheels(const T4 & spd4) {
        T x = (spd4[0] + spd4[3] - spd4[1] - spd4[2]) * T(0.25);
        T y = (spd4[0] + spd4[1] + spd4[2] + spd4[3]) * T(0.25);
        return Vector2_t<T>(x, y);
    }

    T get_spinrate_from_wheels(const T4 & spd4) {
        T temp = config.chassis_height_meter + config.chassis_width_meter;
        return (spd4[1] - spd4[0] + spd4[2] - spd4[3]) / (4 * temp);
    }

    T4  get_wheels_from_status(const Vector2_t<T>& spd, T spinrate) {
        T temp = config.chassis_height_meter + config.chassis_width_meter;
        return {
            spd.y + spd.x - spinrate * temp,
            spd.y - spd.x + spinrate * temp,
            spd.y - spd.x - spinrate * temp,
            spd.y + spd.x + spinrate * temp
        };
    }

public:
    Mecanum4Solver_t(const Config & _config):config(_config) {}

    Ray2D_t<T> forward(const T4 & spd4){
        return {get_velocity_from_wheels(spd4), get_spinrate_from_wheels(spd4)};
    }
    
    Ray2D_t<T> forward(const T w1, const T w2, const T w3, const T w4){
        return forward({w1,w2,w3,w4});
    }

    T4 inverse(const Ray2D_t<T> & pv){
        return get_wheels_from_status(pv.org, pv.rad);
    }

    T4 inverse(const Vector2_t<T> & velocity, const T spinrate){
        return get_wheels_from_status(velocity, spinrate);
    }
};

template<arithmetic T>
OutputStream & operator<<(OutputStream & os, const PoseVelocity2D_t<T> & pv){
    return os << '(' << pv.velocity << ',' << pv.spinrate << ')';
}


}