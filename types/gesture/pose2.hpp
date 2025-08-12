#pragma once

#include "types/vectors/vector2.hpp"

namespace ymd{
template<typename T>
struct Pose2{
    Vector2<T> position;
    T orientation;

    constexpr Pose2():
        position(Vector2<T>()), orientation(){;}

    template<typename U = T>
    constexpr Pose2(const Pose2<U> & other):
        position(other.position), orientation(other.orientation){;}

    template<typename U = T>
    constexpr Pose2(const Vector2<U> & _pos, const U _rad):
        position(_pos), orientation(_rad){;}

    [[nodiscard]] constexpr Pose2<T> forward_move(const T length) const {
        const auto [s, c] = sincos(orientation);
        const auto delta = Vector2<T>{c, s} * length;
        return {
            position + delta,
            orientation
        };
    }

    [[nodiscard]] constexpr Pose2<T> side_move(const T length) const {
        const auto [s, c] = sincos(orientation + q16(PI/2));
        const auto delta = Vector2<T>{c, s} * length;
        return {
            position + delta,
            orientation
        };
    }

    [[nodiscard]] constexpr Pose2<T> revolve_by_radius_and_rotation(
            const T radius, const T rot) const {

        const auto ar = Vector2<T>::from_idenity_rotation(orientation).rotated(
            rot > 0 ? T(PI/2) : T(-PI/2)
        ) * radius;

        const auto org = position + ar;
        const auto delta = (-ar).rotated(rot);
        return {org + delta, orientation + rot};
    }
};

template<typename T>
Pose2(const Vector2<T> & _pos, const T _rad) -> Pose2<T>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Pose2<T> & obj){
    return os << os.brackets<'('>() 
        << obj.position << os.splitter() << 
        obj.orientation << os.brackets<')'>();
}

}