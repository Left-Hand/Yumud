#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"
#include "types/segment2d/Segment2d_t.hpp"

namespace ymd{

template<arithmetic T>
struct Line2D_t{
public:
    T d;
    T rad; 
    // x * -sin(rad) + y * cos(rad) + d = 0

    // y = (x * sin(rad) - d) / cos(rad)
    // x = (y * cos(rad) + d) / sin(rad)

public:
    constexpr Line2D_t(){;}

    constexpr Line2D_t(const Segment2D_t<auto> & seg):
        d(((seg.from).cross(seg.to)) / (seg.to - seg.from).length()), 
        rad(seg.angle()){;}

    constexpr Line2D_t(const Vector2_t<auto> & _from, const Vector2_t<auto> & _to): 
            Line2D_t(Segment2D_t<T>(_from, _to)){;}
            

    // d = p.x * sin(_rad) - p.y * cos(_rad) 
    constexpr Line2D_t(const Vector2_t<auto> & p, const T & _rad): 
        d(p.x * sin(_rad) - p.y * cos(_rad)),rad(_rad){;}

    constexpr Line2D_t(const T & _d, const T & _rad): 
        d(static_cast<T>(_d)),rad(static_cast<T>(_rad)){;}


    __fast_inline constexpr T xfromy(const T & y){
        auto den = sin(rad);
        return den ? (y * cos(rad) + d) / den: T{0};
    }

    __fast_inline constexpr T yfromx(const T & x){
        auto den = cos(rad);
        return den ? (x * sin(rad) - d) / den : T{0};
    }

    __fast_inline constexpr T angle() const {
        //FIXME rounded in [0, TAU]
        return this->rad;
    }

    __fast_inline constexpr Line2D_t<T> abs() const {
        if(T(0) <= rad and rad < T(PI)) return *this; 

        auto m = fposmodp(rad, T(TAU));

        if(d < 0){
            m = m > T(PI) ? m - T(PI) : m + T(PI);
            return {-d, m};
        }else{
            return {d, m};
        }
    }


	__fast_inline constexpr bool operator==(const Line2D_t & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return is_equal_approx(regular.d, other_regular.d) and is_equal_approx(regular.rad, other_regular.rad);
    }

	__fast_inline constexpr bool operator!=(const Line2D_t & other) const{
        return (*this == other) == false; 
    }

    __fast_inline constexpr T distance_to(const Vector2_t<T> & p) const{
        return ABS(this->signed_distance_to(p));
    }
    
    __fast_inline constexpr T signed_distance_to(const Vector2_t<T> & p) const{
        // x * -sin(rad) + y * cos(rad) + d = 0

        return -sin(rad) * p.x + cos(rad) * p.y + d;
    }

    __fast_inline constexpr T angle_to(const Line2D_t<T> & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();

        return other_regular.rad - regular.rad;
    }

    __fast_inline constexpr bool parallel_with(const Line2D_t & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return (is_equal_approx(regular.d, other_regular.d)) and 
                is_equal_approx(fposmodp(other_regular.rad - regular.rad, T(PI)), 0);
    }

    __fast_inline constexpr std::optional<T> distance_with(const Line2D_t<T> & other) const{
        if(not this->parallel_with(other)) return std::nullopt;
        return {this->d - other.d};
    }

    __fast_inline constexpr bool intersects(const Line2D_t<T> & other) const{
        if(this->parallel_with(other)) return false;
        else if(this->operator==(other)) return false;
        return true;
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Line2D_t<T> & other) const{
        if(false == this->intersects(other)) return std::nullopt;
        

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

        return Vector2{num1 * inv_den, num2 * inv_den};
    }

    __fast_inline constexpr Line2D_t<T> rotated(const Vector2_t<T> & p, const T & delta){
        if(this->has_point(p)) return Line2D_t{p, this->rad + delta};
        else{
            //FIXME optimize
            auto rebased = this->rebase(p);
            rebased.rad += delta;
            return rebased;
        }
    }

    __fast_inline constexpr Line2D_t<T> normal(const Vector2_t<T> & p){
        auto new_rad = this->rad + T(PI/2);
        // x * -sin(rad') + y * cos(rad') + d' = 0

        // considering rad' = rad + PI/2

        // x * -cos(rad) + y * -sin(rad) + d' = 0
        // => d' = sin(rad) * y + cos(rad) * x

        // return {sin(this->rad) * p.y + cos(this->rad) * p.x, new_rad}
        return {sin(new_rad) * p.x - cos(new_rad) * p.x, new_rad};
    }

    __fast_inline constexpr Line2D_t<T> rebase(const Vector2_t<T> & p){
        auto regular = this->abs();
        return Line2D_t{regular.distance_to(p), regular.rad};
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> fillet(const Line2D_t<T> & other, const T & radius) const{
        if(false == this->intersects(other)) return std::nullopt;


        //TODO
        return {};
    }
    
    __fast_inline constexpr bool has_point(const Vector2_t<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    __fast_inline constexpr int sign(const Vector2_t<T> & p) const{
        return sign(this->signed_distance_to(Line2D_t(p, this->rad)));
    }

    __fast_inline constexpr std::tuple<T, T, T> abc() const{
        return {-sin(rad), cos(rad), d};
    }


    __fast_inline constexpr bool orthogonal_with(const Line2D_t<T> & other) const {
        return is_equal_approx(fposmodp(other.rad - this->rad, T(PI)), T(PI/2));
        // return fposmodp(other.rad - this->rad, T(PI));
    }

    __fast_inline constexpr Line2D_t<T> mean(const Line2D_t<T> & other) const {
        auto regular = this->abs();
        auto other_regular = other.abs();

        if(regular.paraell_with(other_regular)){
            if(is_equal_approx(regular.rad, other_regular.rad)){
                return *this;
            }else{
                return {0, this->rad};
            }
        }else{
            //FIXME find a simple solve

            auto p = this->intersection(other);
            return Line2D_t<T>(p, this->rad);
        }
    }

    __fast_inline constexpr Vector2_t<T> foot(const Vector2_t<T> & p) const{
        //https://blog.csdn.net/hjxu2016/article/details/111594359

        // x * -sin(rad) + y * cos(rad) + d = 0

        // x = + B * B * x0 - A * B * y0 - A * C / (A * A + B * B)
        // y = - A * B * x0 + A * A * y0 - B * C / (A * A + B * B)

        // considering (A * A + B * B = 1)

        // x = + B * B * x0 - A * B * y0 - A * C
        // y = - A * B * x0 + A * A * y0 - B * C

        auto [x0, y0] = p;

        auto A = -sin(rad);
        auto B = cos(rad);

        auto A2 = A * A;
        auto B2 = B * B;
        auto AB = A * B;

        auto C = d;

        return {B2 * x0 - AB * y0 - A * C, -AB * x0 + A2 * y0 - B * C};
    }

    __fast_inline constexpr Vector2_t<T> mirror(const Vector2_t<T> & p) const {
        return (this->foot(p) * 2) - p;
    }


    __fast_inline constexpr Line2D_t<T> reflect(const Line2D_t<T> & other) const {
        auto res = other.intersection(other);
        if(res){
            //cross
            auto p = res.value();
            auto delta = other.angle_to(*this) * 2;
            return this->rotated(p, delta);
        }else{
            //parallel
            return {};
        }
    }

    __fast_inline constexpr Line2D_t<T> lineplofit(const Vector2_t<T> * begin, const size_t len){
        //TODO
        return {};
    }
    
    __fast_inline constexpr Vector2_t<T> reflect(const Vector2_t<T> & p, const Vector2_t<T> & base) const {
        
        // TODO

        return Vector2_t<T>{0,0};
    }

    __fast_inline constexpr Segment2D_t<T> perpendicular(const Vector2_t<T> & p) const{
        return {p, this->foot(p)};
    }

    __fast_inline constexpr Vector2_t<T> unit() const{
        return {cos(this->rad), sin(this->rad)};
    }

};


__fast_inline OutputStream & operator <<(OutputStream & os, const Line2D_t<auto> & line){
    return os << '(' << line.d << os.splitter() << line.rad << ')';
}

}
