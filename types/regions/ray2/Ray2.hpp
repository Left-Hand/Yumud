#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2/vector2.hpp"
#include "types/regions/line2/Line2.hpp"

namespace ymd{

template<arithmetic T>
struct Ray2_t{
public:
    Vector2<T> org;
    T rad;

public:
    [[nodiscard]] __fast_inline constexpr Ray2_t():
        org(Vector2<T>(0,0)),
        rad(T(0)){;}

    [[nodiscard]] __fast_inline constexpr Ray2_t(const Vector2<T> & _org, const T & _rad): 
            org(static_cast<Vector2<T>>(_org)), rad(static_cast<T>(_rad)){;}

    [[nodiscard]] __fast_inline constexpr Ray2_t(const T & _x, const T & _y, const T & _rad): 
            org(Vector2<T>(_x, _y)), rad(static_cast<T>(_rad)){;}
            
    [[nodiscard]] __fast_inline constexpr Ray2_t(const Vector2<T> & _from, const Vector2<T> & _to): 
            org(static_cast<Vector2<T>>(_from)), rad((_to - _from).angle()){;}

    template<arithmetic U = T>
    [[nodiscard]] __fast_inline constexpr Ray2_t(const std::tuple<U, U, U> & tup) : 
            org((Vector2<T>(std::get<0>(tup), std::get<1>(tup)))),
            rad(std::get<3>(tup)){;}

	[[nodiscard]] __fast_inline constexpr bool operator==(const Ray2_t & other) const{
        return (this->org == other.org) and (this->rad == other.rad);
    }

	[[nodiscard]] __fast_inline constexpr bool is_equal_approx(const Ray2_t & other) const{
        return ymd::is_equal_approx(this->org, other.org) and ymd::is_equal_approx(this->rad, other.rad);
    }

    [[nodiscard]] __fast_inline constexpr bool operator!=(const Ray2_t & other) const{
        return (*this == other) == false; 
    }
    
	[[nodiscard]] __fast_inline constexpr Ray2_t operator + (const Ray2_t & other) const{
        return Ray2_t{this->org + other.org, this->rad + other.rad}.regular();
    }

	[[nodiscard]] __fast_inline constexpr Ray2_t operator - (const Ray2_t & other) const{
        return Ray2_t{this->org - other.org, this->rad - other.rad}.regular();
    }

    [[nodiscard]] __fast_inline constexpr Ray2_t regular() const{
        return Ray2_t{this->org, fposmodp(this->rad, T(TAU))};
    }



    [[nodiscard]] __fast_inline constexpr bool has_point(const Vector2<T> & pt) const{
        return is_equal_approx(distance_to(pt), 0);
    }

    [[nodiscard]] __fast_inline constexpr bool parallel_with(const Ray2_t & other) const{
        return is_equal_approx(this->rad, other.rad);
    }

    [[nodiscard]] __fast_inline constexpr std::optional<Vector2<T>> intersection(const Ray2_t<T> & other) const{
        return this->to_line().intersection(other.to_line());
    }

    [[nodiscard]] __fast_inline constexpr Line2_t<T> normal() const{
        return Line2_t<T>::from_point_and_rad(this->org, this->rad + T(PI/2));
    }

    [[nodiscard]] __fast_inline constexpr Ray2_t<T> rotated(const T & r) const{
        return {this->org, this->rad + r};
    }
    
    [[nodiscard]] __fast_inline constexpr Vector2<T> endpoint(const T & l) const{
        return this->org + Vector2<T>{l, 0}.rotated(this->rad);
    }
    
    [[nodiscard]] __fast_inline constexpr Segment2_t<T> cut(const T & l) const{
        return {this->org, endpoint(l)};
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

    [[nodiscard]] __fast_inline constexpr Line2_t<T> to_line() const{
        return Line2_t<T>::from_point_and_rad(this->org, this->rad);
    }
};


template<size_t Q>
using Ray2q = Ray2_t<iq_t<Q>>;
using Ray2f = Ray2_t<float>;
using Ray2d = Ray2_t<double>;

__inline OutputStream & operator <<(OutputStream & os, const Ray2_t<auto> & ray){
    return os << os.brackets<'('>() << 
        ray.org << os.splitter() << 
        ray.rad << os.brackets<')'>();
}

}
