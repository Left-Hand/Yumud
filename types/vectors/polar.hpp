#pragma once

#include "core/stream/ostream.hpp"
#include "core/math/real.hpp"
#include "types/vectors/vector2.hpp"


namespace ymd{


template<typename T>
requires (std::is_arithmetic_v<T>)
struct Polar{
    T radius;
    Angle<T> angle;

    constexpr Polar from_vec2(const Vec2<T> & v){
        return Polar(v.length(), v.angle());
    }

    constexpr Vec2<T> to_vec2() const {
        // const auto [s,c] = sincos(angle);
        // return Vec2<T>(radius * c, radius * c);
        return Vec2<T>::from_angle_and_length(angle, radius);
    } 

    friend OutputStream & operator<<(OutputStream & os, const Polar & p){
        return os << os.brackets<'('>() << p.radius 
        << os.splitter() << p.angle << os.brackets<')'>();
    }
};

template<arithmetic T>
Polar() -> Polar<T>;

}