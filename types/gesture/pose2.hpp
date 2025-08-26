#pragma once

#include "types/vectors/vector2.hpp"
#include "core/utils/angle.hpp"

namespace ymd{
template<typename T>
struct Pose2{
    Vec2<T> position;
    Angle<T> orientation;


    template<typename U = T>
    constexpr Pose2(const Pose2<U> & other):
        position(other.position), orientation(other.orientation){;}

    template<typename U = T>
    constexpr Pose2(const Vec2<U> & _pos, const Angle<U> _rad):
        position(_pos), orientation(_rad){;}

    [[nodiscard]] constexpr Pose2<T> forward_move(const T length) const {
        const auto delta = Vec2<T>::from_angle_and_length(orientation, length);
        return Pose2<T>{
            position + delta,
            orientation
        };
    }

    [[nodiscard]] constexpr Pose2<T> side_move(const T length) const {
        const auto delta = Vec2<T>::from_angle_and_length(
            orientation + Angle<T>::QUARTER_LAP,
            length
        );
        return Pose2<T>{
            position + delta,
            orientation
        };
    }

    [[nodiscard]] constexpr Pose2<T> revolve_by_radius_and_rotation(
            const T radius, const Angle<T> angle) const {

        const auto ar = Vec2<T>::from_angle(orientation).rotated(
            angle > Angle<T>::ZERO ? Angle<T>::QUARTER_LAP : Angle<T>::NEG_QUARTER_LAP
        ) * radius;

        const auto org = position + ar;
        const auto delta = (-ar).rotated(angle);
        return Pose2<T>{
            org + delta, 
            orientation + angle
        };
    }
};

template<typename T>
Pose2(const Vec2<T> & _pos, const T _rad) -> Pose2<T>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Pose2<T> & obj){
    return os << os.brackets<'('>() 
        << obj.position << os.splitter() << 
        obj.orientation << os.brackets<')'>();
}

}