#pragma once

#include "core/math/real.hpp"
#include "types/vector2/vector2.hpp"
#include "types/Line2/Line2.hpp"

namespace ymd{

template<arithmetic T>
struct Ray2_t{
public:
    Vector2_t<T> org;
    T rad;

public:
    __fast_inline constexpr Ray2_t(){;}

    __fast_inline constexpr Ray2_t(const Vector2_t<T> & _org, const T & _rad): 
            org(static_cast<Vector2_t<T>>(_org)), rad(static_cast<T>(_rad)){;}

    __fast_inline constexpr Ray2_t(const T & _x, const T & _y, const T & _rad): 
            org(Vector2_t<T>(_x, _y)), rad(static_cast<T>(_rad)){;}
            
    __fast_inline constexpr Ray2_t(const Vector2_t<T> & _from, const Vector2_t<T> & _to): 
            org(static_cast<Vector2_t<T>>(_from)), rad((_to - _from).angle()){;}

    template<arithmetic U = T>
    __fast_inline constexpr Ray2_t(const std::tuple<U, U, U> & tup) : 
            org((Vector2_t<T>(std::get<0>(tup), std::get<1>(tup)))),
            rad(std::get<3>(tup)){;}

	__fast_inline constexpr bool operator==(const Ray2_t & other) const{
        return is_equal_approx(this->org == other.org) and is_equal_appro(this->rad, other.rad);
    }

	__fast_inline constexpr Ray2_t operator + (const Ray2_t & other) const{
        return Ray2_t{this->org + other.org, this->rad + other.rad}.regular();
    }

	__fast_inline constexpr Ray2_t operator - (const Ray2_t & other) const{
        return Ray2_t{this->org - other.org, this->rad - other.rad}.regular();
    }

    __fast_inline constexpr Ray2_t regular() const{
        return Ray2_t{this->org, fposmodp(this->rad, T(TAU))};
    }

	__fast_inline constexpr bool operator!=(const Ray2_t & other) const{
        return (*this == other) == false; 
    }

    __fast_inline constexpr bool has_point(const Vector2_t<T> & pt) const{
        return is_equal_approx(distance_to(pt), 0);
    }

    __fast_inline constexpr bool parallel_with(const Ray2_t & other) const{
        return is_equal_approx(this->rad, other.rad);
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Ray2_t<T> & other) const{
        // if(this->parallel_with(other)) return std::nullopt;

        // //TODO
        // // return {0,0};
        return Line2_t<T>(*this).intersection(Line2_t<T>(other));
    }

    __fast_inline constexpr Line2_t<T> normal() const{
        return Line2_t<T>(this->org, this->rad + T(PI/2));
    }

    __fast_inline constexpr Ray2_t<T> rotated(const T & r) const{
        return {this->org, this->rad + r};
    }
    
    __fast_inline constexpr Vector2_t<T> endpoint(const T & l) const{
        return this->org + Vector2_t<T>{l, 0}.rotated(this->rad);
    }
    
    __fast_inline constexpr Segment2D_t<T> cut(const T & l) const{
        return {this->org, endpoint(l)};
    }

    __fast_inline constexpr T distance_to(const Vector2_t<T> & pt) const{
        //TODO
        
        return 0;
    }

    __fast_inline constexpr std::tuple<T, T, T> abc() const{

        //(y - y0) = tan(rad) * (x - x0)
        // -tan(rad) * x + y - y0 + tan(rad) * x0 = 0

        // auto t = tan(this->rad);
        // return {-t, q, t * org.x - org.y};

        // -sin(rad) * x + cos(rad) * y - cos(rad) * y0 + sin(rad) * x0 = 0

        auto s = sin(this->rad);
        auto c = cos(this->rad);

        return {-s, c, - c * org.y + s * org.x};
    }

    operator Line2_t<T>() const{
        return Line2_t<T>(this->org, this->rad);
    }
};


__inline OutputStream & operator <<(OutputStream & os, const Ray2_t<auto> & ray){
    return os << os.brackets<'('>() << 
        ray.org << os.splitter() << 
        ray.rad << os.brackets<')'>();
}

}
