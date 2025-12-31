#pragma once

#include "algebra/gesture/twist2.hpp"

namespace ymd::robots::kinematics{
template<arithmetic T>
struct [[nodiscard]] Mecanum4Kinematics final{
public:
    using T4 = std::tuple<T, T, T, T>;
    
    struct Config{
        //底盘宽度(米)
        const T chassis_width;
        //底盘长度(米)
        const T chassis_height;
    };

    constexpr explicit Mecanum4Kinematics(const Config & cfg):cfg_(cfg) {}

    constexpr Twist2<T> forward(const T4 & spd4) const {
        return {get_velocity_from_wheels(spd4), get_spinrate_from_wheels(spd4)};
    }
    
    constexpr T4 inverse(const Twist2<T> & pv) const {
        return get_wheels_from_status(pv.linear, pv.angular);
    }
private:
    const Config cfg_;

    constexpr Vec2<T> get_velocity_from_wheels(const T4 & spd4) const {
        T s0 = std::get<0>(spd4);
        T s1 = std::get<1>(spd4);
        T s2 = std::get<2>(spd4);
        T s3 = std::get<3>(spd4);

        T x = (s0 + s3 - s1 - s2) * T(0.25);
        T y = (s0 + s1 + s2 + s3) * T(0.25);
        return Vec2<T>(x, y);
    }

    constexpr T get_spinrate_from_wheels(const T4 & spd4) const {
        T temp = (cfg_.chassis_height + cfg_.chassis_width) / 2;
        T s0 = std::get<0>(spd4);
        T s1 = std::get<1>(spd4);
        T s2 = std::get<2>(spd4);
        T s3 = std::get<3>(spd4);
        return (s1 - s0 + s2 - s3) / (4 * temp);
    }

    constexpr T4  get_wheels_from_status(const Vec2<T>& spd, T spinrate) const {
        T temp = (cfg_.chassis_height + cfg_.chassis_width) / 2;
        return {
            spd.y + spd.x - spinrate * temp,
            spd.y - spd.x + spinrate * temp,
            spd.y - spd.x - spinrate * temp,
            spd.y + spd.x + spinrate * temp
        };
    }
};



}