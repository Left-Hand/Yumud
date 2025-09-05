#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/segment2.hpp"
#include "core/utils/Option.hpp"

namespace ymd{

template<arithmetic T>
struct Line2{
public:
    T d;
    Angle<T> orientation; 
private:
    constexpr Line2(const T _d, const Angle<T> angle): 
        d(static_cast<T>(_d)),orientation((angle)){;}

public:

    [[nodiscard]] constexpr Line2(const Segment2<auto> & seg):
        d(((seg.start).cross(seg.stop)) / (seg.stop - seg.start).length()), 
        orientation(seg.angle()){;}

    [[nodiscard]] constexpr Line2(const Vec2<auto> & _from, const Vec2<auto> & _to): 
            Line2(Segment2<T>(_from, _to)){;}
            

    // d = p.x * sin(angle) - p.y * cos(angle) 
    [[nodiscard]] static constexpr Line2 from_point_and_angle(
        const Vec2<auto> & p, 
        const Angle<T> angle
    ){
        const auto [s,c] = angle.sincos();
        return Line2{p.x * s - p.y * c, angle};
    }

    [[nodiscard]] static constexpr Line2 from_dist_and_angle(const T _d, const Angle<T> angle){
        return Line2{_d, angle};
    }


    [[nodiscard]] __fast_inline constexpr
    T x_at_y(const T y){
        const auto [s,c] = orientation.sincos();
        return s ? (y * c + d) / s: T{0};
    }

    [[nodiscard]] __fast_inline constexpr
    T y_at_x(const T x){
        const auto [s,c] = orientation.sincos();
        auto den = c;
        return den ? (x * s - d) / den : T{0};
    }


    [[nodiscard]] __fast_inline constexpr
    Line2<T> abs() const {
        if(0_deg <= orientation and orientation < 180_deg) return *this; 

        auto m = fposmod(orientation.to_radians(), T(TAU));

        if(d < 0){
            m = m > T(PI) ? m - T(PI) : m + T(PI);
            return {-d, Angle<T>::from_radians(m)};
        }else{
            return {d, Angle<T>::from_radians(m)};
        }
    }


	[[nodiscard]] __fast_inline constexpr
    bool operator==(const Line2 & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return is_equal_approx(regular.d, other_regular.d) 
        and is_equal_approx(regular.orientation, other_regular.orientation);
    }

	[[nodiscard]] __fast_inline constexpr
    bool operator!=(const Line2 & other) const{
        return (*this == other) == false; 
    }

    [[nodiscard]] __fast_inline constexpr
    T distance_to(const Vec2<T> & p) const{
        return ABS(this->signed_distance_to(p));
    }
    
    [[nodiscard]] __fast_inline constexpr
    T signed_distance_to(const Vec2<T> & p) const{
        // x * -sin(orientation) + y * cos(orientation) + d = 0
        const auto [s,c] = orientation.sincos();
        return -s * p.x + c * p.y + d;
    }

    [[nodiscard]] __fast_inline constexpr
    T angle_between(const Line2<T> & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();

        return other_regular.orientation - regular.orientation;
    }

    [[nodiscard]] __fast_inline constexpr
    bool parallel_with(const Line2 & other) const{
        auto regular = this->abs();
        auto other_regular = other.abs();
        return (is_equal_approx(regular.d, other_regular.d)) and 
                is_equal_approx(fposmod((other_regular.orientation - regular.orientation).to_radians(), T(PI)), T(0));
    }

    [[nodiscard]] __fast_inline constexpr
    Option<T> distance_with(const Line2<T> & other) const{
        if(not this->parallel_with(other)) return std::nullopt;
        return Some<T>({this->d - other.d});
    }

    [[nodiscard]] __fast_inline constexpr
    bool intersects(const Line2<T> & other) const{
        if(this->parallel_with(other)) return false;
        else if(this->operator==(other)) return false;
        return true;
    }

    [[nodiscard]] __fast_inline constexpr
    Option<Vec2<T>> intersection(const Line2<T> & other) const{
        if(unlikely(false == this->intersects(other))) return None;
        

        //https://www.cnblogs.com/junlin623/p/17640554.html

        //A1x + B1y + C1 = 0
        //A2x + B2y + C2 = 0

        //x0 = (- B2 * C1 + B1 * C2) / (A1 * B2 - A2 * B1)
        //y0 = (- A1 * C2 + A2 * C1) / (A1 * B2 - A2 * B1)

        const auto [a1, b1, c1] = this->abc();
        const auto [a2, b2, c2] = other.abc();

        const auto num1 = c2 * b1 - c1 * b2;
        const auto num2 = c1 * a2 - c2 * a1;
        const auto den = a1 * b2 - a2 * b1;
        const auto inv_den = T(1) / den;

        return Some(Vec2<real_t>{num1 * inv_den, num2 * inv_den});
    }

    [[nodiscard]] __fast_inline constexpr
    Line2<T> rotated(const Vec2<T> & p, const Angle<T> angle){
        if(this->has_point(p)) return Line2::from_point_and_angle(p, this->orientation + angle);
        else{
            //FIXME optimize
            auto rebased = this->rebase(p);
            rebased.orientation = rebased.orientation + angle;
            return rebased;
        }
    }

    [[nodiscard]] __fast_inline constexpr
    Line2<T> normal(const Vec2<T> & p){
        const auto next_angle = this->orientation + Angle<T>::QUARTER_LAP;
        const auto [s,c] = next_angle.sincos();
        return {s * p.x - c * p.x, next_angle};
    }


    [[nodiscard]] __fast_inline constexpr
    Line2<T> rebase(const Vec2<T> & p){
        auto regular = this->abs();
        return Line2{regular.distance_to(p), regular.orientation};
    }

    [[nodiscard]] __fast_inline constexpr
    Option<Vec2<T>> fillet(const Line2<T> & other, const T radius) const{
        if(false == this->intersects(other)) return std::nullopt;


        //TODO
        return {};
    }
    
    [[nodiscard]] __fast_inline constexpr
    bool has_point(const Vec2<T> & p) const{
        return is_equal_approx(distance_to(p), T(0));
    }

    [[nodiscard]] __fast_inline constexpr
    int sign(const Vec2<T> & p) const{
        return sign(this->signed_distance_to(Line2(p, this->orientation)));
    }

    //直线标准方程 ax + by + c = 0的三个系数
    [[nodiscard]] __fast_inline constexpr
    std::tuple<T, T, T> abc() const{
        const auto [s,c] = orientation.sincos();
        return {-s, c, d};
    }


    //是否与另一条直线正交
    [[nodiscard]] __fast_inline constexpr
    bool is_orthogonal_with(const Line2<T> & other) const {
        return is_equal_approx(fposmod(
            (other.orientation - this->orientation).to_radians()
        , T(PI)), T(PI/2));
        // return fposmod(other.orientation - this->orientation, T(PI));
    }

    [[nodiscard]] __fast_inline constexpr
    Line2<T> mean(const Line2<T> & other) const {
        auto regular = this->abs();
        auto other_regular = other.abs();

        if(regular.is_paraell_with(other_regular)){
            if(is_equal_approx(regular.orientation, other_regular.orientation)){
                return *this;
            }else{
                return {0, this->orientation};
            }
        }else{
            //FIXME find a simple solve

            auto p = this->intersection(other);
            return Line2<T>(p, this->orientation);
        }
    }

    //计算直线关于某个点的垂足
    [[nodiscard]] __fast_inline constexpr
    Vec2<T> foot_of(const Vec2<T> & p) const{
        //https://blog.csdn.net/hjxu2016/article/details/111594359

        // x * -sin(orientation) + y * cos(orientation) + d = 0

        // x = + B * B * x0 - A * B * y0 - A * C / (A * A + B * B)
        // y = - A * B * x0 + A * A * y0 - B * C / (A * A + B * B)

        // considering (A * A + B * B = 1)

        // x = + B * B * x0 - A * B * y0 - A * C
        // y = - A * B * x0 + A * A * y0 - B * C

        auto [x0, y0] = p;

        auto [s, c] = orientation.sincos();

        auto A2 = s * s;
        auto B2 = c * c;
        auto AB = -s * c;

        auto C = d;

        return {B2 * x0 - AB * y0 + s * C, -AB * x0 + A2 * y0 - c * C};
    }

    [[nodiscard]] __fast_inline constexpr
    Vec2<T> mirror(const Vec2<T> & p) const {
        return (this->foot_of(p) * 2) - p;
    }


    [[nodiscard]] __fast_inline constexpr
    Line2<T> reflect(const Line2<T> & other) const {
        auto res = other.intersection(other);
        if(res){
            //cross
            auto p = res.value();
            auto delta = other.angle_between(*this) * 2;
            return this->rotated(p, delta);
        }else{
            //parallel
            return {};
        }
    }

    [[nodiscard]] __fast_inline constexpr
    Line2<T> lineplofit(const Vec2<T> * begin, const size_t len){
        //TODO
        return {};
    }
    
    [[nodiscard]] __fast_inline constexpr
    Vec2<T> reflect(const Vec2<T> & p, const Vec2<T> & base) const {
        
        // TODO

        return Vec2<T>{0,0};
    }

    [[nodiscard]] __fast_inline constexpr
    Segment2<T> perpendicular(const Vec2<T> & p) const{
        return {p, this->foot_of(p)};
    }
};


__fast_inline OutputStream & operator <<(OutputStream & os, const Line2<auto> & line){
    return os << '(' << line.d << os.splitter() << line.orientation << ')';
}

}
