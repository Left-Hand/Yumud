#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"
#include "types/line2d/Line2D_t.hpp"
#include "types/segment2d/Segment2d_t.hpp"

namespace yumud{

template<arithmetic T>
struct Line2D_t{
public:
    T d;
    T a; 
    // x * -sin(a) + y * cos(a) + d = 0

public:
    constexpr Line2D_t(){;}

    constexpr Line2D_t(const Segment2D_t<auto> & seg):
        d(seg.diff().length()), a(seg.diff().angle()){;}

    constexpr Line2D_t(const Vector2_t<auto> & _from, const Vector2_t<auto> & _to): 
            Line2D_t(Segment2D_t<T>(_from, _to)){;}
            

    // d = p.x * sin(_a) - p.y * cos(_a) 
    constexpr Line2D_t(const Vector2_t<auto> & p, const T _a): 
        d(p.x * sin(_a) - p.y * cos(_a)),a(_a){;}


	__fast_inline constexpr bool operator==(const Line2D_t & other) const{
        return is_equal_approx(d, other.d) and is_equal_approx(a, other.a);
    }

	__fast_inline constexpr bool operator!=(const Line2D_t & other) const{
        return (*this == other) == false; 
    }

    __fast_inline constexpr bool parrel_with(const Line2D_t & other) const{
        return is_equal_approx(d, other.d) and (not is_equal_approx(a, other.a));
    }


    __fast_inline constexpr T distance_to(const Vector2_t<T> & p) const{
        auto res = this->distance_to(Line2D_t{p, this->a});
        if(res){
            return res.value();
        }else{
            return 0;
        }
    }

    __fast_inline constexpr std::optional<T> distance_to(const Line2D_t<T> & other) const{
        if(not this->parrel_with(other)) return std::nullopt;
        return {this->d - other.d};
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Line2D_t<T> & other) const{
        if(this->parrel_with(other)) return std::nullopt;
        else if(this->operator==(other)) return std::nullopt;
        

        //https://www.cnblogs.com/junlin623/p/17640554.html

        //A1x + B1y + C1 = 0
        //A2x + B2y + C2 = 0

        //x0 = (- B2 * C1 + B1 * C2) / (A1 * B2 - A2 * B1)
        //y0 = (- A1 * C2 + A2 * C1) / (A1 * B2 - A2 * B1)

        auto [a1, b1, c1] = this->abc();
        auto [a2, b2, c2] = other.abc();

        auto num1 = c2 * b1 - c1 * b2;
        auto num2 = c1 * a2 - c2 * a1;
        auto den = a1 * b2 - a2 * b1;
        auto inv_den = T(1) / den;

        return {num1 * inv_den, num2 * inv_den};
    }
    
    __fast_inline constexpr bool has_point(const Vector2_t<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    __fast_inline constexpr int sign(const Vector2_t<T> & p) const{
        return sign(this->distance_to(Line2D_t(p, this->a)));
    }

    __fast_inline constexpr std::tuple<T, T, T> abc() const{
        return {-sin(a), cos(a), d};
    }
};

}
