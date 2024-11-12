#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"

namespace yumud{

template<arithmetic T>
struct Segment2D_t{
public:
    Vector2_t<T> from;
    Vector2_t<T> to;

public:
    constexpr Segment2D_t(){;}

    constexpr Segment2D_t(const Vector2_t<auto> & _from, const Vector2_t<auto> & _to): 
            from(static_cast<Vector2_t<T>>(_from)), to(static_cast<Vector2_t<T>>(to)){;}

    template<arithmetic U = T>
    constexpr Segment2D_t(const std::tuple<U, U, U, U> & tup) : 
            from((Vector2_t<T>(std::get<0>(tup), std::get<1>(tup)))),
            to((Vector2_t<T>(std::get<2>(tup), std::get<3>(tup)))){;}

    __fast_inline constexpr const Vector2_t<T> & operator [](const size_t idx) const {
        if(idx > 2) HALT;
        return *(&from + idx);
    }

    __fast_inline constexpr Vector2_t<T> & operator [](const size_t idx){
        if(idx > 2) HALT;
        return *(&from + idx);
    }

	__fast_inline constexpr bool operator==(const Segment2D_t & other) const{
        return from == other.from and to == other.to;
    }

	__fast_inline constexpr bool operator!=(const Segment2D_t & other) const{
        return (*this == other) == false; 
    }

    __fast_inline constexpr Vector2_t<T> get_center() const{
        return (this->from + this->to)/2;
    }

    __fast_inline constexpr T distance_to(const Vector2_t<T> & p) const{
        const auto diff1 = from - p;
        const auto diff2 = to - p;
        const auto diff3 = to - from;

        if(diff1.dot(diff3) > 0){
            return diff1.length();
        }else if(diff2.dot(diff3) < 0){
            return diff2.length();
        }else{
        return ((diff1).cross(diff2) / 2) / (diff3).length();
        }
    }

    __fast_inline constexpr bool has_point(const Vector2_t<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    __fast_inline constexpr int sign(const Vector2_t<T> & p) const{
        return sign((from - p).cross(to - p));
    }

    __fast_inline constexpr bool parrel_with(const Segment2D_t & other){
        return is_equal_approx(this->diff().cross(other.diff()), 0);
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Segment2D_t<T> & other) const{
        if(this->parrel_with(other)) return std::nullopt;

        //TODO
        return {this->d - other.d};
    }

    __fast_inline constexpr Vector2_t<T> diff() const{
        return to - from;
    }
};

}
