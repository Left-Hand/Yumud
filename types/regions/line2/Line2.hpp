#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2/vector2.hpp"
#include "types/regions/segment2/Segment2.hpp"

namespace ymd{

template<arithmetic T>
struct Line2{
public:
    T d;
    T rad; 
private:
    constexpr Line2(const T & _d, const T & _rad): 
        d(static_cast<T>(_d)),rad(static_cast<T>(_rad)){;}

public:
    [[nodiscard]] constexpr Line2():
        d(0),
        rad(0){;}

    [[nodiscard]] constexpr Line2(const Segment2<auto> & seg):
        d(((seg.from).cross(seg.to)) / (seg.to - seg.from).length()), 
        rad(seg.angle()){;}

    [[nodiscard]] constexpr Line2(const Vector2<auto> & _from, const Vector2<auto> & _to): 
            Line2(Segment2<T>(_from, _to)){;}
            

    // d = p.x * sin(_rad) - p.y * cos(_rad) 
    [[nodiscard]] static constexpr Line2 from_point_and_rad(const Vector2<auto> & p, const T & _rad){
        const auto [s,c] = sincos(_rad);
        return Line2{p.x * s - p.y * c, _rad};
    }

    [[nodiscard]] static constexpr Line2 from_dist_and_rad(const T & _d, const T & _rad){
        return Line2{_d, _rad};
    }


    [[nodiscard]] __fast_inline constexpr T xfromy(const T & y){
        auto den = sin(rad);
        return den ? (y * cos(rad) + d) / den: T{0};
    }

    [[nodiscard]] __fast_inline constexpr T yfromx(const T & x){
        auto den = cos(rad);
        return den ? (x * sin(rad) - d) / den : T{0};
    }

    [[nodiscard]] __fast_inline constexpr T angle() const {
        //FIXME rounded in [0, TAU]
        return this->rad;
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> abs() const {
        if(T(0) <= rad and rad < T(PI)) return *this; 

        auto m = fposmodp(rad, T(TAU));

        if(d < 0){
            m = m > T(PI) ? m - T(PI) : m + T(PI);
            return {-d, m};
        }else{
            return {d, m};
        }
    }


	[[nodiscard]] __fast_inline constexpr bool operator==(const Line2 & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return is_equal_approx(regular.d, other_regular.d) and is_equal_approx(regular.rad, other_regular.rad);
    }

	[[nodiscard]] __fast_inline constexpr bool operator!=(const Line2 & other) const{
        return (*this == other) == false; 
    }

    [[nodiscard]] __fast_inline constexpr T distance_to(const Vector2<T> & p) const{
        return ABS(this->signed_distance_to(p));
    }
    
    [[nodiscard]] __fast_inline constexpr T signed_distance_to(const Vector2<T> & p) const{
        // x * -sin(rad) + y * cos(rad) + d = 0
        const auto [s,c] = sincos(rad);
        return -s * p.x + c * p.y + d;
    }

    [[nodiscard]] __fast_inline constexpr T angle_to(const Line2<T> & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();

        return other_regular.rad - regular.rad;
    }

    [[nodiscard]] __fast_inline constexpr bool parallel_with(const Line2 & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return (is_equal_approx(regular.d, other_regular.d)) and 
                is_equal_approx(fposmodp(other_regular.rad - regular.rad, T(PI)), T(0));
    }

    [[nodiscard]] __fast_inline constexpr std::optional<T> distance_with(const Line2<T> & other) const{
        if(not this->parallel_with(other)) return std::nullopt;
        return {this->d - other.d};
    }

    [[nodiscard]] __fast_inline constexpr bool intersects(const Line2<T> & other) const{
        if(this->parallel_with(other)) return false;
        else if(this->operator==(other)) return false;
        return true;
    }

    [[nodiscard]] __fast_inline constexpr std::optional<Vector2<T>> intersection(const Line2<T> & other) const{
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

        return Vector2<real_t>{num1 * inv_den, num2 * inv_den};
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> rotated(const Vector2<T> & p, const T & delta){
        if(this->has_point(p)) return Line2::from_point_and_rad(p, this->rad + delta);
        else{
            //FIXME optimize
            auto rebased = this->rebase(p);
            rebased.rad += delta;
            return rebased;
        }
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> normal(const Vector2<T> & p){
        auto new_rad = this->rad + T(PI/2);
        return {sin(new_rad) * p.x - cos(new_rad) * p.x, new_rad};
    }


    [[nodiscard]] __fast_inline constexpr Line2<T> rebase(const Vector2<T> & p){
        auto regular = this->abs();
        return Line2{regular.distance_to(p), regular.rad};
    }

    [[nodiscard]] __fast_inline constexpr std::optional<Vector2<T>> fillet(const Line2<T> & other, const T & radius) const{
        if(false == this->intersects(other)) return std::nullopt;


        //TODO
        return {};
    }
    
    [[nodiscard]] __fast_inline constexpr bool has_point(const Vector2<T> & p) const{
        return is_equal_approx(distance_to(p), T(0));
    }

    [[nodiscard]] __fast_inline constexpr int sign(const Vector2<T> & p) const{
        return sign(this->signed_distance_to(Line2(p, this->rad)));
    }

    //直线标准方程 ax + by + c = 0的三个系数
    [[nodiscard]] __fast_inline constexpr std::tuple<T, T, T> abc() const{
        return {-sin(rad), cos(rad), d};
    }


    //是否与另一条直线正交
    [[nodiscard]] __fast_inline constexpr bool is_orthogonal_with(const Line2<T> & other) const {
        return is_equal_approx(fposmodp(other.rad - this->rad, T(PI)), T(PI/2));
        // return fposmodp(other.rad - this->rad, T(PI));
    }

    [[nodiscard]] __fast_inline constexpr Line2<T> mean(const Line2<T> & other) const {
        auto regular = this->abs();
        auto other_regular = other.abs();

        if(regular.is_paraell_with(other_regular)){
            if(is_equal_approx(regular.rad, other_regular.rad)){
                return *this;
            }else{
                return {0, this->rad};
            }
        }else{
            //FIXME find a simple solve

            auto p = this->intersection(other);
            return Line2<T>(p, this->rad);
        }
    }

    //计算直线关于某个点的垂足
    [[nodiscard]] __fast_inline constexpr Vector2<T> foot_of(const Vector2<T> & p) const{
        //https://blog.csdn.net/hjxu2016/article/details/111594359

        // x * -sin(rad) + y * cos(rad) + d = 0

        // x = + B * B * x0 - A * B * y0 - A * C / (A * A + B * B)
        // y = - A * B * x0 + A * A * y0 - B * C / (A * A + B * B)

        // considering (A * A + B * B = 1)

        // x = + B * B * x0 - A * B * y0 - A * C
        // y = - A * B * x0 + A * A * y0 - B * C

        auto [x0, y0] = p;

        auto [s, c] = sincos(rad);

        auto A2 = s * s;
        auto B2 = c * c;
        auto AB = -s * c;

        auto C = d;

        return {B2 * x0 - AB * y0 + s * C, -AB * x0 + A2 * y0 - c * C};
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> mirror(const Vector2<T> & p) const {
        return (this->foot_of(p) * 2) - p;
    }


    [[nodiscard]] __fast_inline constexpr Line2<T> reflect(const Line2<T> & other) const {
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

    [[nodiscard]] __fast_inline constexpr Line2<T> lineplofit(const Vector2<T> * begin, const size_t len){
        //TODO
        return {};
    }
    
    [[nodiscard]] __fast_inline constexpr Vector2<T> reflect(const Vector2<T> & p, const Vector2<T> & base) const {
        
        // TODO

        return Vector2<T>{0,0};
    }

    [[nodiscard]] __fast_inline constexpr Segment2<T> perpendicular(const Vector2<T> & p) const{
        return {p, this->foot_of(p)};
    }

    [[nodiscard]] __fast_inline constexpr Vector2<T> unit() const{
        return {cos(this->rad), sin(this->rad)};
    }

};


__fast_inline OutputStream & operator <<(OutputStream & os, const Line2<auto> & line){
    return os << '(' << line.d << os.splitter() << line.rad << ')';
}

}
