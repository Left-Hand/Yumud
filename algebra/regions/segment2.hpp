#pragma once

#include "core/math/real.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/Rect2.hpp"

namespace ymd{

template<arithmetic T>
struct Segment2{
public:
    Vec2<T> start;
    Vec2<T> stop;

public:

    template<typename U>
    [[nodiscard]] constexpr explicit Segment2(const Segment2<U> & segment):
        start(Vec2<T>(segment.start)), stop(Vec2<T>(segment.stop)){;}

    [[nodiscard]] constexpr Segment2(const Vec2<auto> & _start, const Vec2<auto> & _stop): 
            start(static_cast<Vec2<T>>(_start)), stop(static_cast<Vec2<T>>(_stop)){;}

    template<arithmetic U = T>
    [[nodiscard]] constexpr Segment2(const std::tuple<U, U, U, U> & tup) : 
            start((Vec2<T>(std::get<0>(tup), std::get<1>(tup)))),
            stop((Vec2<T>(std::get<2>(tup), std::get<3>(tup)))){;}

    [[nodiscard]] __fast_inline constexpr 
    const Vec2<T> & operator [](const size_t idx) const {
        if(idx > 2) __builtin_unreachable();
        return *(&start + idx);
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> & operator [](const size_t idx){
        if(idx > 2) __builtin_unreachable();
        return *(&start + idx);
    }

	[[nodiscard]] __fast_inline constexpr 
    bool operator==(const Segment2 & other) const{
        return start == other.start and stop == other.stop;
    }

	[[nodiscard]] __fast_inline constexpr 
    bool operator!=(const Segment2 & other) const{
        return (*this == other) == false; 
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> get_center() const{
        return (this->start + this->stop)/2;
    }

    [[nodiscard]] __fast_inline constexpr 
    T distance_to(const Vec2<T> & p) const{
        const auto diff1 = start - p;
        const auto diff2 = stop - p;
        const auto diff3 = stop - start;

        if(diff1.dot(diff3) > 0){
            return diff1.length();
        }else if(diff2.dot(diff3) < 0){
            return diff2.length();
        }else{
        return ((diff2).cross(diff1)) / (diff3).length();
        }
    }

    [[nodiscard]] __fast_inline constexpr 
    bool has_point(const Vec2<T> & p) const{
        return is_equal_approx(distance_to(p), 0);
    }

    [[nodiscard]] __fast_inline constexpr 
    int sign(const Vec2<T> & p) const{
        return sign((start - p).cross(stop - p));
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_parallel_with(const Segment2 & other){
        return is_equal_approx(this->diff().cross(other.diff()), 0);
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_orthogonal_with(const Segment2 & other){
        return is_equal_approx(this->diff().dot(other.diff()), 0);
    }

    [[nodiscard]] __fast_inline constexpr 
    std::optional<Vec2<T>> intersection(const Segment2<T> & other) const{
        if(this->is_parallel_with(other)) return std::nullopt;
        else if(this->operator==(other)) return std::nullopt;
        

        //https://www.cnblogs.com/junlin623/p/17640554.html

        //A1x + B1y + C1 = 0
        //A2x + B2y + C2 = 0

        //x0 = (- B2 * C1 + B1 * C2) / (A1 * B2 - A2 * B1)
        //y0 = (- A1 * C2 + A2 * C1) / (A1 * B2 - A2 * B1)

        return {this->d - other.d};
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> diff() const{
        return stop - start;
    }

    [[nodiscard]] __fast_inline constexpr 
    std::tuple<T, T, T> abc() const{

        //https://www.cnblogs.com/sailJs/p/17802652.html

        //Ax1 + By1 + C1 = 0 
        //Ax2 + By2 + C2 = 0 

        //a=y2-y1, b=x1-x2, c=y1*x2-x1*y2

        return {start.y - stop.y, stop.x - start.x, start.cross(stop)};
    }

    [[nodiscard]] __fast_inline constexpr 
    T length() const {
        return (stop - start).length();
    }

    [[nodiscard]] __fast_inline constexpr 
    T length_squared() const{
        return (stop - start).length_squared();
    }

    [[nodiscard]] __fast_inline constexpr 
    Angular<T> angle() const {
        return (stop - start).angle();
    }

    [[nodiscard]] __fast_inline constexpr 
    Rect2<T> bounding_box() const{
        const auto points = std::to_array({start, stop});
        return Rect2<T>::from_minimal_bounding_box(points);
    }

    [[nodiscard]] __fast_inline constexpr 
    T x_at_y(const T y) const{
        if (start.y == stop.y) {
            return start.x; // For horizontal lines, return the x-coordinate
        }
        return start.x + (stop.x - start.x) * (y - start.y) / (stop.y - start.y);
    }

    [[nodiscard]] __fast_inline constexpr 
    Segment2 swap_if_inverted() const {
        if(start.y > stop.y){
            return {stop, start};
        }
        return *this;
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_horizontal() const{
        return start.y == stop.y;
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_vertical() const{
        return start.x == stop.x;
    }

    [[nodiscard]] __fast_inline constexpr 
    Segment2 swap() const {
        return {stop, start};
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> midpoint() const {
        return {(start.x + stop.x) / 2, (start.y + stop.y) / 2};
    }

    [[nodiscard]] __fast_inline constexpr 
    T x_delta_per_y(const T y_unit) const {
        static_assert(not std::is_integral_v<T>);
        if (start.y == stop.y) {
            return T{0}; // No change in x per y unit for horizontal lines
        }
        return (stop.x - start.x) * y_unit / (stop.y - start.y);
    }
};


using Segment2f = Segment2<float>;
using Segment2d = Segment2<double>;

using Segment2i = Segment2<int>;
using Segment2u = Segment2<uint>;

__inline OutputStream & operator <<(OutputStream & os, const Segment2<auto> & seg){
    return os << os.brackets<'('>() << 
        seg.start << os.splitter() << 
        seg.stop << os.brackets<')'>();
}

}
