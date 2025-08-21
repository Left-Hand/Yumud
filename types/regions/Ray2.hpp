#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/Line2.hpp"

namespace ymd{

template<arithmetic T>
struct Ray2{
public:
    Vec2<T> org;
    T rad;

public:
    [[nodiscard]] __fast_inline constexpr Ray2():
        org(Vec2<T>(0,0)),
        rad(T(0)){;}

    [[nodiscard]] __fast_inline constexpr Ray2(const Vec2<T> & _org, const T & _rad): 
            org(static_cast<Vec2<T>>(_org)), rad(static_cast<T>(_rad)){;}
            
    [[nodiscard]] __fast_inline constexpr Ray2(const Vec2<T> & _from, const Vec2<T> & _to): 
            org(static_cast<Vec2<T>>(_from)), rad((_to - _from).angle()){;}

    template<arithmetic U = T>
    [[nodiscard]] __fast_inline constexpr Ray2(const std::tuple<U, U, U> & tup) : 
            org((Vec2<T>(std::get<0>(tup), std::get<1>(tup)))),
            rad(std::get<3>(tup)){;}

	[[nodiscard]] __fast_inline constexpr bool operator==(const Ray2 & other) const{
        return (this->org == other.org) and (this->rad == other.rad);
    }

	[[nodiscard]] __fast_inline constexpr bool is_equal_approx(const Ray2 & other) const{
        return ymd::is_equal_approx(this->org, other.org) and ymd::is_equal_approx(this->rad, other.rad);
    }

    [[nodiscard]] __fast_inline constexpr bool operator!=(const Ray2 & other) const{
        return (*this == other) == false; 
    }
    
	[[nodiscard]] __fast_inline constexpr Ray2 operator + (const Ray2 & other) const{
        return Ray2{this->org + other.org, this->rad + other.rad}.regular();
    }

	[[nodiscard]] __fast_inline constexpr Ray2 operator - (const Ray2 & other) const{
        return Ray2{this->org - other.org, this->rad - other.rad}.regular();
    }

    [[nodiscard]] __fast_inline constexpr Ray2 regular() const{
        return Ray2{this->org, fposmod(this->rad, T(TAU))};
    }



    [[nodiscard]] __fast_inline constexpr bool has_point(const Vec2<T> & pt) const{
        return is_equal_approx(distance_to(pt), 0);
    }

    [[nodiscard]] __fast_inline constexpr bool parallel_with(const Ray2 & other) const{
        return is_equal_approx(this->rad, other.rad);
    }

    [[nodiscard]] __fast_inline constexpr Option<Vec2<T>> intersection(const Ray2<T> & other) const{
        return this->to_line().intersection(other.to_line());
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> normal() const{
        return Line2<T>::from_point_and_angle(this->org, this->rad + T(PI/2));
    }

    [[nodiscard]] __fast_inline constexpr Ray2<T> rotated(const T & r) const{
        return {this->org, this->rad + r};
    }
    
    [[nodiscard]] __fast_inline constexpr Vec2<T> endpoint_at_length(const T & l) const{
        return this->org + Vec2<T>{l, 0}.rotated(this->rad);
    }
    
    [[nodiscard]] __fast_inline constexpr Segment2<T> cut_by_length(const T & l) const{
        return {this->org, endpoint_at_length(l)};
    }

    [[nodiscard]] __fast_inline constexpr std::tuple<T, T, T> abc() const{

        //(y - y0) = tan(rad) * (x - x0)
        // -tan(rad) * x + y - y0 + tan(rad) * x0 = 0

        // auto t = tan(this->rad);
        // return {-t, q, t * org.x - org.y};

        // -sin(rad) * x + cos(rad) * y - cos(rad) * y0 + sin(rad) * x0 = 0

        const auto [s, c] = sincos(this->rad);

        return {-s, c, - c * org.y + s * org.x};
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> to_line() const{
        return Line2<T>::from_point_and_angle(this->org, this->rad);
    }
};


using Ray2f = Ray2<float>;
using Ray2d = Ray2<double>;

__inline OutputStream & operator <<(OutputStream & os, const Ray2<auto> & ray){
    return os << os.brackets<'('>() << 
        ray.org << os.splitter() << 
        ray.rad << os.brackets<')'>();
}

}
