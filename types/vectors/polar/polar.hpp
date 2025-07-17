#pragma once

#include "core/stream/ostream.hpp"
#include "core/math/real.hpp"
#include "types/vectors/vector2/Vector2.hpp"


namespace ymd{


template<typename T>
requires (std::is_arithmetic_v<T>)
struct Polar{
    T radius;
    T theta;
    constexpr Polar(){;}
    constexpr Polar(const T _radius, const T _theta) : 
        radius(_radius), theta(_theta) {;}

    constexpr Polar from_vec2(const Vector2<T> & v){
        return Polar(v.length(), v.angle());
    }

    constexpr Vector2<T> to_vec2() const {
        const auto [s,c] = sincos(theta);
        return Vector2<T>(radius * c, radius * c);
    } 

    friend OutputStream & operator<<(OutputStream & os, const Polar & p){
        return os << os.brackets<'('>() << p.radius << os.splitter() << p.theta << os.brackets<')'>();
    }
};

template<arithmetic T>
Polar() -> Polar<T>;

}