#pragma once

#include "prelude.hpp"
#include "line_iter.hpp"

namespace ymd{

template<typename T>
struct Triangle2{
    std::array<Vec2<T>, 3> points;


    constexpr Vec2<T> top() const {
        const auto min_y = MIN(points[0].y, points[1].y, points[2].y);
        for (const auto& point : points) {
            if (point.y == min_y) {
                return point;
            }
        }
        return points[0]; // fallback
    }

    constexpr Vec2<T> bottom() const {
        const auto max_y = MAX(points[0].y, points[1].y, points[2].y);
        for (const auto& point : points) {
            if (point.y == max_y) {
                return point;
            }
        }
        return points[0]; // fallback
    }

    constexpr Vec2<T> middle() const {
        const auto y0 = points[0].y, y1 = points[1].y, y2 = points[2].y;
        const auto min_y = MIN(y0, y1, y2);
        const auto max_y = MAX(y0, y1, y2);
        
        if (y0 != min_y && y0 != max_y) return points[0];
        if (y1 != min_y && y1 != max_y) return points[1];
        if (y2 != min_y && y2 != max_y) return points[2];
        
        // If all points have same y or only two distinct y values, return one of the non-extreme points
        if (y0 == min_y && y1 == max_y) return points[2];
        if (y0 == max_y && y1 == min_y) return points[2];
        return points[1];
    }

    constexpr bool is_clockwise() const{
        return (points[1] - points[0]).cross(points[2] - points[0]) < 0;
    }

    constexpr bool is_counter_clockwise() const {
        return (points[1] - points[0]).cross(points[2] - points[0]) > 0;
    }

    constexpr T area() const {
        return (points[1] - points[0]).cross(points[2] - points[0]) / 2;
    }

    constexpr bool contains_point(const Vec2<T>& p) const {
        const auto& a = points[0];
        const auto& b = points[1];
        const auto& c = points[2];
        
        // 计算重心坐标
        const Vec2<T> v0 = b - a;
        const Vec2<T> v1 = c - a;
        const Vec2<T> v2 = p - a;
        
        const T dot00 = v0.dot(v0);
        const T dot01 = v0.dot(v1);
        const T dot02 = v0.dot(v2);
        const T dot11 = v1.dot(v1);
        const T dot12 = v1.dot(v2);
        
        // 计算重心坐标参数
        const T inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
        const T u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
        const T v = (dot00 * dot12 - dot01 * dot02) * inv_denom;
        
        // 检查点是否在三角形内部
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }

    constexpr Triangle2<T> to_sorted_by_y() const {
        Triangle2<T> result = *this;
        auto& p = result.points; // 引用别名提高可读性
        
        // 最优的比较网络 for 3 elements
        if (p[0].y > p[1].y) std::swap(p[0], p[1]);
        if (p[1].y > p[2].y) std::swap(p[1], p[2]);
        if (p[0].y > p[1].y) std::swap(p[0], p[1]);
        
        return result;
    }

    constexpr Rect2<T> bounding_box() const {
        return Rect2<T>::from_minimal_bounding_box(points);
    }

    constexpr Vec2<T> operator[](const size_t index) const{
        return points[index];
    }

    constexpr Vec2<T> & operator[](const size_t index){
        return points[index];
    }

    constexpr Vec2<T> compute_centroid() const{
        return (points[0] + points[1] + points[2]) / 3;
    }

    constexpr std::tuple<T, T, T> compute_barycentric_2d(const Vec2<T> p)
    {
        const auto [xp, yp] = p;
        const T xa = points[0].x, ya = points[0].y;
        const T xb = points[1].x, yb = points[1].y;
        const T xc = points[2].x, yc = points[2].y;
        const T gamma = ((xb - xa) * (yp - ya) - (xp - xa) * (yb - ya)) / 
                    ((xb - xa) * (yc - ya) - (xc - xa) * (yb - ya));
        const T beta = (xp - xa - gamma * (xc - xa)) / (xb - xa);
        const T alpha = 1 - beta - gamma;
        return {alpha,beta,gamma};
    }

private:
    static constexpr bool is_points_clockwise(const std::span<const Vec2<T>> points) {
        if (points.size() < 3) return false;
        
        T cross_sum = T{0};
        for (size_t i = 0; i < points.size(); ++i) {
            const size_t j = (i + 1) % points.size();
            cross_sum += (points[j].x - points[i].x) * (points[j].y + points[i].y);
            // 或者如果 Vec2<T> 有 cross() 方法：cross_sum += points[i].cross(points[j]);
        }
        
        // 正和：逆时针，负和：顺时针
        return cross_sum > 0;  // 修正符号
    }

};


template <typename T>
struct is_placed_t<Triangle2<T>>: std::true_type {};



template<typename T>
struct TriangleIterator {
public:
    using Point = Vec2<T>;
    using Segment = Segment2<T>;
    using LineIterator = LineDDAIterator<T>;

    constexpr TriangleIterator(const Triangle2<T>& sorted_tri_)
        :
            is_mid_at_right_(is_point_at_right(sorted_tri_.points[0], sorted_tri_.points[2], sorted_tri_.points[1])),
            current_y_(sorted_tri_.points[0].y),
            mid_y_(sorted_tri_.points[1].y),
            stop_y_(sorted_tri_.points[2].y),
            top_left_iter_(is_mid_at_right_ ? 
                LineIterator(Segment{sorted_tri_.points[0], sorted_tri_.points[2]}) : 
                LineIterator(Segment{sorted_tri_.points[0], sorted_tri_.points[1]})),
            top_right_iter_(is_mid_at_right_ ? 
                LineIterator(Segment{sorted_tri_.points[0], sorted_tri_.points[1]}) : 
                LineIterator(Segment{sorted_tri_.points[0], sorted_tri_.points[2]})),
            bottom_iter_(is_mid_at_right_ ? 
                LineIterator(Segment{sorted_tri_.points[1], sorted_tri_.points[2]}) : 
                LineIterator(Segment{sorted_tri_.points[2], sorted_tri_.points[1]}))
    {}

    __fast_inline constexpr bool has_next() const {
        return current_y_ < stop_y_;
    }

    __fast_inline constexpr Range2u16 current_filled() const {
        auto & self = *this;
        return Range2u16::from_start_and_stop(
        // return Range2u16::from_start_and_stop_unchecked(
            floor_cast<uint16_t>(left_iter(self).x()), 
            ceil_cast<uint16_t>(right_iter(self).x())
        );
    }

    __fast_inline constexpr std::tuple<Range2u16, Range2u16> left_and_right() const {
        auto & self = *this;
        return {
            left_iter(self).x_range(),
            right_iter(self).x_range(),
        };
    }

    __fast_inline constexpr void advance() {
        auto & self = *this;
        if (current_y_ >= stop_y_) return;
        ++current_y_;

        left_iter(self).advance();
        right_iter(self).advance();
    }

private:
    /* 
    ....A
    ....|\
    ....|.\
    ....|..\
    ....|...P
    ....|
    ....B  
    */

    __fast_inline constexpr bool is_point_at_right(
        const Point& a, 
        const Point& b, 
        const Point& p
    ) const {
        Vec2<T> ab = b - a;
        Vec2<T> ap = p - a;
        return ap.is_counter_clockwise_to(ab);
    }


    //古人模仿deducing this历史
    template<typename Self>
    static constexpr auto & left_iter(Self && self){
        if (self.is_mid_at_right_) {
            return self.top_left_iter_;
        } else {
            return (self.current_y_ <= self.mid_y_) ? 
                self.top_left_iter_: self.bottom_iter_;
        }
    }

    template<typename Self>
    static constexpr auto & right_iter(Self && self){
        if (not self.is_mid_at_right_) {
            return self.top_right_iter_;
        } else {
            return (self.current_y_ <= self.mid_y_) ? 
                self.top_right_iter_: self.bottom_iter_;
        }
    }

    bool is_mid_at_right_;
    uint16_t current_y_;
    uint16_t mid_y_;
    uint16_t stop_y_;
    LineIterator top_left_iter_;
    LineIterator top_right_iter_;
    LineIterator bottom_iter_;
};


template<std::integral T>
struct DrawDispatchIterator<Triangle2<T>> {
    using Triangle = Triangle2<T>;
    using Iterator = TriangleIterator<T>;

    constexpr DrawDispatchIterator(const Triangle & triangle):
        iter_(triangle.to_sorted_by_y()){;}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return iter_.has_next();
    }

    // 推进到下一行
    constexpr void forward() {
        iter_.advance();
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_filled(Target& target, const Color& color) {
        // 绘制当前行的范围
        const auto x_range = iter_.current_filled();
        auto res = target.fill_x_range(x_range, color);
        if (res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    // 空心绘制（对于线段来说和填充一样）
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        // 绘制当前行的范围
        const auto [left_x_range, right_x_range] = iter_.left_and_right();
        if(const auto res = target.fill_x_range(left_x_range, color);
            res.is_err()) return res;
        if(const auto res = target.fill_x_range(right_x_range, color);
            res.is_err()) return res;
        return Ok();
    }

private:
    TriangleIterator<T> iter_;
};
}