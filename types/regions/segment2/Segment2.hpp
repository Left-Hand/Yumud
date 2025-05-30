#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2/vector2.hpp"

namespace ymd{

template<arithmetic T>
struct Segment2_t{
public:
    Vector2<T> from;
    Vector2<T> to;

public:
    [[nodiscard]] constexpr Segment2_t(){;}

    [[nodiscard]] constexpr Segment2_t(const Vector2<auto> & _from, const Vector2<auto> & _to): 
            from(static_cast<Vector2<T>>(_from)), to(static_cast<Vector2<T>>(_to)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Segment2_t(const std::tuple<U, U, U, U> & tup) : 
            from((Vector2<T>(std::get<0>(tup), std::get<1>(tup)))),
            to((Vector2<T>(std::get<2>(tup), std::get<3>(tup)))){;}

    [[nodiscard]] __fast_inline constexpr const Vector2<T> & operator [](const size_t idx) const {
        if(idx > 2) HALT;
        return *(&from + idx);
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> & operator [](const size_t idx){
        if(idx > 2) HALT;
        return *(&from + idx);
    }

	[[nodiscard]] __fast_inline constexpr bool operator==(const Segment2_t & other) const{
        return from == other.from and to == other.to;
    }

	[[nodiscard]] __fast_inline constexpr bool operator!=(const Segment2_t & other) const{
        return (*this == other) == false; 
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> get_center() const{
        return (this->from + this->to)/2;
    }

    [[nodiscard]] __fast_inline constexpr T distance_to(const Vector2<T> & p) const{
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

    [[nodiscard]] __fast_inline constexpr bool has_point(const Vector2<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    [[nodiscard]] __fast_inline constexpr int sign(const Vector2<T> & p) const{
        return sign((from - p).cross(to - p));
    }

    [[nodiscard]] __fast_inline constexpr bool parallel_with(const Segment2_t & other){
        return is_equal_approx(this->diff().cross(other.diff()), 0);
    }

    [[nodiscard]] __fast_inline constexpr bool orthogonal_with(const Segment2_t & other){
        return is_equal_approx(this->diff().dot(other.diff()), 0);
    }

    [[nodiscard]] __fast_inline constexpr std::optional<Vector2<T>> intersection(const Segment2_t<T> & other) const{
        if(this->parallel_with(other)) return std::nullopt;
        else if(this->operator==(other)) return std::nullopt;
        

        //https://www.cnblogs.com/junlin623/p/17640554.html

        //A1x + B1y + C1 = 0
        //A2x + B2y + C2 = 0

        //x0 = (- B2 * C1 + B1 * C2) / (A1 * B2 - A2 * B1)
        //y0 = (- A1 * C2 + A2 * C1) / (A1 * B2 - A2 * B1)

        return {this->d - other.d};
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> diff() const{
        return to - from;
    }

    [[nodiscard]] __fast_inline constexpr std::tuple<T, T, T> abc() const{

        //https://www.cnblogs.com/sailJs/p/17802652.html

        //Ax1 + By1 + C1 = 0 
        //Ax2 + By2 + C2 = 0 

        //a=y2-y1, b=x1-x2, c=y1*x2-x1*y2

        return {from.y - to.y, to.x - from.x, from.cross(to)};
    }

    [[nodiscard]] __fast_inline constexpr T length() const {
        return (to - from).length();
    }

    [[nodiscard]] __fast_inline constexpr T length_squared() const{
        return (to - from).length_squared();
    }

    [[nodiscard]] __fast_inline constexpr T angle() const {
        return (to - from).angle();
    }
};


template<size_t Q>
using Segment2q = Segment2_t<iq_t<Q>>;
using Segment2f = Segment2_t<float>;
using Segment2d = Segment2_t<double>;

using Segment2i = Segment2_t<int>;
using Segment2u = Segment2_t<uint>;

__inline OutputStream & operator <<(OutputStream & os, const Segment2_t<auto> & seg){
    return os << os.brackets<'('>() << 
        seg.from << os.splitter() << 
        seg.to << os.brackets<')'>();
}

}
