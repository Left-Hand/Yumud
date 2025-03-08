#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"

namespace ymd{

template<arithmetic T>
struct Segment2D_t{
public:
    Vector2_t<T> from;
    Vector2_t<T> to;

public:
    constexpr Segment2D_t(){;}

    constexpr Segment2D_t(const Vector2_t<auto> & _from, const Vector2_t<auto> & _to): 
            from(static_cast<Vector2_t<T>>(_from)), to(static_cast<Vector2_t<T>>(_to)){;}

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
        return ((diff2).cross(diff1)) / (diff3).length();
        }
    }

    __fast_inline constexpr bool has_point(const Vector2_t<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    __fast_inline constexpr int sign(const Vector2_t<T> & p) const{
        return sign((from - p).cross(to - p));
    }

    __fast_inline constexpr bool parallel_with(const Segment2D_t & other){
        return is_equal_approx(this->diff().cross(other.diff()), 0);
    }

    __fast_inline constexpr bool orthogonal_with(const Segment2D_t & other){
        return is_equal_approx(this->diff().dot(other.diff()), 0);
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Segment2D_t<T> & other) const{
        if(this->parallel_with(other)) return std::nullopt;
        else if(this->operator==(other)) return std::nullopt;
        

        //https://www.cnblogs.com/junlin623/p/17640554.html

        //A1x + B1y + C1 = 0
        //A2x + B2y + C2 = 0

        //x0 = (- B2 * C1 + B1 * C2) / (A1 * B2 - A2 * B1)
        //y0 = (- A1 * C2 + A2 * C1) / (A1 * B2 - A2 * B1)

        return {this->d - other.d};
    }

    __fast_inline constexpr Vector2_t<T> diff() const{
        return to - from;
    }

    __fast_inline constexpr std::tuple<T, T, T> abc() const{

        //https://www.cnblogs.com/sailJs/p/17802652.html

        //Ax1 + By1 + C1 = 0 
        //Ax2 + By2 + C2 = 0 

        //a=y2-y1, b=x1-x2, c=y1*x2-x1*y2

        return {from.y - to.y, to.x - from.x, from.cross(to)};
    }

    __fast_inline constexpr T length() const {
        return (to - from).length();
    }

    __fast_inline constexpr T length_squared() const{
        return (to - from).length_squared();
    }

    __fast_inline constexpr T angle() const {
        return (to - from).angle();
    }
};


__inline OutputStream & operator <<(OutputStream & os, const Segment2D_t<auto> & seg){
    return os << os.brackets<'('>() << 
        seg.from << os.splitter() << 
        seg.to << os.brackets<')'>();
}

}
