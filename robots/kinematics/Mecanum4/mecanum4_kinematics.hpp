#pragma once

#include "types/gesture/twist2.hpp"

namespace ymd{
template<arithmetic T>
class Mecanum4Kinematics{
public:
    using T4 = std::tuple<T, T, T, T>;
    
    struct Config{
        const T chassis_width_meter;
        const T chassis_height_meter;
    };

protected:
    const Config & config;

    constexpr Vec2<T> get_velocity_from_wheels(const T4 & spd4) {
        T s0 = std::get<0>(spd4);
        T s1 = std::get<1>(spd4);
        T s2 = std::get<2>(spd4);
        T s3 = std::get<3>(spd4);

        T x = (s0 + s3 - s1 - s2) * T(0.25);
        T y = (s0 + s1 + s2 + s3) * T(0.25);
        return Vec2<T>(x, y);
    }

    constexpr T get_spinrate_from_wheels(const T4 & spd4) {
        T temp = (config.chassis_height_meter + config.chassis_width_meter) / 2;
        T s0 = std::get<0>(spd4);
        T s1 = std::get<1>(spd4);
        T s2 = std::get<2>(spd4);
        T s3 = std::get<3>(spd4);
        return (s1 - s0 + s2 - s3) / (4 * temp);
    }

    constexpr T4  get_wheels_from_status(const Vec2<T>& spd, T spinrate) {
        T temp = (config.chassis_height_meter + config.chassis_width_meter) / 2;
        return {
            spd.y + spd.x - spinrate * temp,
            spd.y - spd.x + spinrate * temp,
            spd.y - spd.x - spinrate * temp,
            spd.y + spd.x + spinrate * temp
        };
    }

public:
    constexpr Mecanum4Kinematics(const Config & _config):config(_config) {}

    constexpr Twist2<T> forward(const T4 & spd4){
        return {get_velocity_from_wheels(spd4), get_spinrate_from_wheels(spd4)};
    }
    
    constexpr T4 inverse(const Twist2<T> & pv){
        return get_wheels_from_status(pv.linear, pv.angular);
    }
};



}