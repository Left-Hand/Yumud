#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/Line2.hpp"

namespace ymd{

template<arithmetic T>
struct Ray2{
public:
    Vec2<T> org;
    Angle<T> orientation;

public:
    [[nodiscard]] __fast_inline constexpr Ray2():
        org(Vec2<T>(0,0)),
        orientation(T(0)){;}

    [[nodiscard]] __fast_inline constexpr Ray2(const Vec2<T> & _org, const Angle<T> & angle): 
            org(static_cast<Vec2<T>>(_org)), orientation(angle){;}
            
    [[nodiscard]] __fast_inline constexpr Ray2(const Vec2<T> & _from, const Vec2<T> & _to): 
            org(static_cast<Vec2<T>>(_from)), orientation((_to - _from).angle()){;}

    template<arithmetic U = T>
    [[nodiscard]] __fast_inline constexpr Ray2(const std::tuple<U, U, U> & tup) : 
            org((Vec2<T>(std::get<0>(tup), std::get<1>(tup)))),
            orientation(std::get<3>(tup)){;}

	[[nodiscard]] __fast_inline constexpr bool operator==(const Ray2 & other) const{
        return (this->org == other.org) and (this->orientation == other.orientation);
    }

	[[nodiscard]] __fast_inline constexpr bool is_equal_approx(const Ray2 & other) const{
        return ymd::is_equal_approx(this->org, other.org) and ymd::is_equal_approx(this->orientation, other.orientation);
    }

    [[nodiscard]] __fast_inline constexpr bool operator!=(const Ray2 & other) const{
        return (*this == other) == false; 
    }
    
	[[nodiscard]] __fast_inline constexpr Ray2 operator + (const Ray2 & other) const{
        return Ray2{this->org + other.org, this->orientation + other.orientation}.regular();
    }

	[[nodiscard]] __fast_inline constexpr Ray2 operator - (const Ray2 & other) const{
        return Ray2{this->org - other.org, this->orientation - other.orientation}.regular();
    }

    [[nodiscard]] __fast_inline constexpr Ray2 regular() const{
        return Ray2{this->org, fposmod(this->orientation, T(TAU))};
    }



    [[nodiscard]] __fast_inline constexpr bool has_point(const Vec2<T> & pt) const{
        return is_equal_approx(distance_to(pt), 0);
    }

    [[nodiscard]] __fast_inline constexpr bool parallel_with(const Ray2 & other) const{
        return is_equal_approx(this->orientation, other.orientation);
    }

    [[nodiscard]] __fast_inline constexpr Option<Vec2<T>> intersection(const Ray2<T> & other) const{
        return this->to_line().intersection(other.to_line());
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> normal() const{
        return Line2<T>::from_point_and_angle(this->org, this->orientation + Angle<T>::from_radians(T(PI/2)));
    }

    [[nodiscard]] __fast_inline constexpr Ray2<T> rotated(const Angle<T> angle) const{
        return {this->org, this->orientation + angle};
    }
    
    [[nodiscard]] __fast_inline constexpr Vec2<T> endpoint_at_length(const T & l) const{
        return this->org + Vec2<T>{l, 0}.rotated(this->orientation);
    }
    
    [[nodiscard]] __fast_inline constexpr Segment2<T> cut_by_length(const T & l) const{
        return {this->org, endpoint_at_length(l)};
    }

    [[nodiscard]] __fast_inline constexpr std::tuple<T, T, T> abc() const{

        //(y - y0) = tan(orientation) * (x - x0)
        // -tan(orientation) * x + y - y0 + tan(orientation) * x0 = 0

        // auto t = tan(this->orientation);
        // return {-t, q, t * org.x - org.y};

        // -sin(orientation) * x + cos(orientation) * y - cos(orientation) * y0 + sin(orientation) * x0 = 0

        const auto [s, c] = sincos(this->orientation);

        return {-s, c, - c * org.y + s * org.x};
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> to_line() const{
        return Line2<T>::from_point_and_angle(this->org, this->orientation);
    }
};


using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;

__inline OutputStream & operator <<(OutputStream & os, const Ray2<auto> & ray){
    return os << os.brackets<'('>() << 
        ray.org << os.splitter() << 
        ray.orientation << os.brackets<')'>();
}

}
