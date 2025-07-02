#pragma once

#include "types/vectors/vector2/Vector2.hpp"

namespace ymd{
template<typename T>
struct Pose2{
    Vector2<T> pos;
    T rad;

    template<typename U = T>
    constexpr Pose2(const Pose2<U> & other):
        pos(other.pos), rad(other.rad){;}

    template<typename U = T>
    constexpr Pose2(const Vector2<U> & _pos, const U _rad):
        pos(_pos), rad(_rad){;}

    [[nodiscard]] constexpr Pose2<T> forward_move(const T length) const {
        const auto [s, c] = sincos(rad);
        const auto delta = Vector2<T>{c, s} * length;
        return {
            pos + delta,
            rad
        };
    }

    [[nodiscard]] constexpr Pose2<T> side_move(const T length) const {
        const auto [s, c] = sincos(rad + q16(PI/2));
        const auto delta = Vector2<T>{c, s} * length;
        return {
            pos + delta,
            rad
        };
    }

    [[nodiscard]] constexpr Pose2<T> revolve_by_radius_and_rotation(
            const T radius, const T rot) const {

        const auto ar = Vector2<T>::from_idenity_rotation(rad).rotated(
            rot > 0 ? T(PI/2) : T(-PI/2)
        ) * radius;

        const auto org = pos + ar;
        const auto delta = (-ar).rotated(rot);
        return {org + delta, rad + rot};
    }
};

template<typename T>
Pose2(const Vector2<T> & _pos, const T _rad) -> Pose2<T>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Pose2<T> & obj){
    return os << os.brackets<'('>() 
        << obj.pos << os.splitter() << 
        obj.rad << os.brackets<')'>();
}

}