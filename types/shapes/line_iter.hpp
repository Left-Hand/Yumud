#pragma once

#include "prelude.hpp"
#include "triangle2.hpp"
#include "circle2.hpp"

namespace ymd{

template<std::integral T>
struct BresenhamIterator {
    using Point = Vec2<T>;
    using Segment = Segment2<T>;
    using SignedT = std::make_signed_t<T>;

    constexpr BresenhamIterator(const Segment& segment)
        : start_(segment.start),
            stop_(segment.stop),
            current_y_(std::min(segment.start.y, segment.stop.y)),
            max_y_(std::max(segment.start.y, segment.stop.y)) {
        
        // 初始化 Bresenham 算法参数
        SignedT dx = static_cast<SignedT>(stop_.x) - static_cast<SignedT>(start_.x);
        SignedT dy = static_cast<SignedT>(stop_.y) - static_cast<SignedT>(start_.y);
        
        dx_ = std::abs(dx);
        dy_ = std::abs(dy);
        sx_ = (dx > 0) ? 1 : -1;
        sy_ = (dy > 0) ? 1 : -1;
        
        // 初始化 Bresenham 状态
        current_x_ = start_.x;
        bresenham_y_ = start_.y;
        err_ = dx_ - dy_;
        
        // 如果起始点不是最小y值，需要推进到正确的行
        if (start_.y != current_y_) {
            advance_to_row(current_y_);
        }
    }

    constexpr bool has_next() const {
        return current_y_ <= max_y_;
    }

    constexpr Range2<T> current() const {
        if (!has_next()) return Range2<T>{0, 0};
        return Range2<T>::from_start_and_length(current_x_, 1);
    }

    constexpr void advance() {
        if (!has_next()) return;
        
        current_y_++;
        if (current_y_ <= max_y_) {
            advance_to_row(current_y_);
        }
    }

private:
    constexpr void advance_to_row(T target_y) {
        // 推进 Bresenham 算法直到达到目标行
        while (bresenham_y_ != target_y && has_next()) {
            SignedT e2 = 2 * err_;
            
            if (e2 > -static_cast<SignedT>(dy_)) {
                err_ -= static_cast<SignedT>(dy_);
                current_x_ += sx_;
            }
            
            if (e2 < static_cast<SignedT>(dx_)) {
                err_ += static_cast<SignedT>(dx_);
                bresenham_y_ += sy_;
                
                // 检查是否超过了目标行
                if ((sy_ > 0 && bresenham_y_ > target_y) || 
                    (sy_ < 0 && bresenham_y_ < target_y)) {
                    break;
                }
            }
        }
    }

private:
    Point start_;
    Point stop_;
    T current_y_;      // 当前处理的行号
    T current_x_;      // 当前行的 x 坐标
    T bresenham_y_;    // Bresenham 算法当前的 y 坐标
    T max_y_;          // 最大 y 坐标
    T dx_;             // x 方向绝对值
    T dy_;             // y 方向绝对值
    SignedT sx_;       // x 方向符号
    SignedT sy_;       // y 方向符号
    SignedT err_;      // 误差项
};


template<typename T>
struct LineDDAIterator{
    using Point = Vec2<uint16_t>;
    using Segment = Segment2<uint16_t>;

    constexpr LineDDAIterator(const Segment& segment){
        auto & self = *this;
        const auto fixed_segment = segment.swap_if_inverted();

        self.x_step_ = Segment2<q16>(fixed_segment).x_delta_per_y(1);
        self.current_x_ = fixed_segment.start.x;
        self.current_y_ = fixed_segment.start.y;
        self.stop_y_ = fixed_segment.stop.y;
    }

    __fast_inline constexpr bool has_next() const {
        return current_y_ < stop_y_;
    }

    __fast_inline constexpr q16 x() const {
        return current_x_;
    }

    __fast_inline constexpr Range2u16 x_range() const{
        const q16 a = x();
        const q16 b = a + x_step();
        if(a < b){
            return Range2u16::from_start_and_stop_unchecked(
                floor_cast<uint16_t>(a), 
                ceil_cast<uint16_t>(b)
            );
        }else{
            return Range2u16::from_start_and_stop_unchecked(
                floor_cast<uint16_t>(b), 
                ceil_cast<uint16_t>(a)
            );
        }
    }

    __fast_inline constexpr void advance(){
        current_y_ += 1;    
        current_x_ += x_step_;
    }

    __fast_inline constexpr q16 x_step() const {
        return x_step_;
    }
private:
    q16 x_step_;
    q16 current_x_;
    uint16_t current_y_;
    uint16_t stop_y_;
};

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



template<typename T>
struct CircleBresenhamIterator{
public:
    constexpr CircleBresenhamIterator(const Circle2<T> & circle):
        x0_(circle.center.x),
        err_(1 - 2 * circle.radius),
        y_(-circle.radius),
        radius_(circle.radius),
        radius_squ_(square(circle.radius))
    {
        replace_x();
    }

    constexpr void advance(){
        y_++;
        replace_x();
    }

    constexpr bool has_next() const {
        return y_ < radius_;
    }

    constexpr Range2u16 x_range() const{
        return Range2u16{x0_ + x_, x0_ - x_};
        // return Range2u16{x0_ - 3, x0_ + 3};
    }

    constexpr std::tuple<Range2u16, Range2u16> left_and_right() const {
        return {
            Range2u16{x0_ + x_, x0_ + x_ + 1},
            Range2u16{x0_ - x_, x0_ - x_ + 1}
        };
    }

    constexpr bool is_y_at_edge() const {
        return y_ == (radius_ - 1) || y_ == (-radius_);
    }

    constexpr bool is_y_at_zero() const {
        return y_ == 0;
    }
private:
    int16_t x0_;
    int16_t err_;
    int16_t y_;
    
    int16_t radius_;
    uint16_t radius_squ_;

    int16_t x_ = 0;


    constexpr void replace_x(){
        x_ = -radius_;
        while(x_ * x_ + y_ * y_ > radius_squ_){
            x_++;
        }
    }

};


                // let delta = Point::new(*x, y) * 2 - self.center_2x;
                // (delta.length_squared() as u32) < self.threshold

}