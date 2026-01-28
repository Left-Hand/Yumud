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

        self.x_step_ = Segment2<iq16>(fixed_segment).x_delta_per_y(1);
        self.current_x_ = fixed_segment.start.x;
        self.current_y_ = fixed_segment.start.y;
        self.stop_y_ = fixed_segment.stop.y;
    }

    __fast_inline constexpr bool has_next() const {
        return current_y_ < stop_y_;
    }

    __fast_inline constexpr iq16 x() const {
        return current_x_;
    }

    __fast_inline constexpr Range2<int16_t> x_range() const{
        const iq16 a = x();
        const iq16 b = a + x_step();
        if(a < b){
            return Range2<int16_t>::from_start_and_stop_unchecked(
                math::floor_cast<uint16_t>(a), 
                math::ceil_cast<uint16_t>(b)
            );
        }else{
            return Range2<int16_t>::from_start_and_stop_unchecked(
                math::floor_cast<uint16_t>(b), 
                math::ceil_cast<uint16_t>(a)
            );
        }
    }

    __fast_inline constexpr void advance(){
        current_y_ += 1;    
        current_x_ += x_step_;
    }

    __fast_inline constexpr iq16 x_step() const {
        return x_step_;
    }
private:
    iq16 x_step_;
    iq16 current_x_;
    uint16_t current_y_;
    uint16_t stop_y_;
};




template<typename T>
struct CircleBresenhamIterator{
public:
    constexpr CircleBresenhamIterator(const Circle2<T> & circle):
        x0_(circle.center.x),
        err_(1 - 2 * static_cast<int16_t>(circle.radius)),
        y_offset_(-static_cast<int16_t>(circle.radius)),
        radius_squ_(static_cast<uint32_t>(math::square(static_cast<int32_t>(circle.radius)))),
        radius_(circle.radius)
    {
        replace_x();
    }

    constexpr void advance(){
        y_offset_++;
        replace_x();
    }

    [[nodiscard]] constexpr bool has_next() const {
        return y_offset_ < radius_;
    }

    [[nodiscard]] constexpr Range2<int16_t> x_range() const{
        return Range2<int16_t>{x0_ + x_offset_, x0_ - x_offset_};
    }

    constexpr std::tuple<Range2<int16_t>, Range2<int16_t>> left_and_right() const {
        return {
            Range2<int16_t>{x0_ + x_offset_, x0_ + x_offset_ + 1},
            Range2<int16_t>{x0_ - x_offset_, x0_ - x_offset_ + 1}
        };
    }

    constexpr bool is_y_at_edge() const {
        return y_offset_ == (radius_ - 1) || y_offset_ == (-radius_);
    }

    constexpr bool is_y_at_zero() const {
        return y_offset_ == 0;
    }
private:
    int16_t x0_;
    int16_t err_;
    int16_t x_offset_ = 0;
    int16_t y_offset_;
    
    uint32_t radius_squ_;
    int16_t radius_;



    constexpr void replace_x(){
        x_offset_ = -radius_;
        while(true){
            const uint32_t coord_squ = static_cast<uint32_t>(
                math::square(static_cast<int32_t>(x_offset_)) 
                + math::square(static_cast<int32_t>(y_offset_))
            );
            if(coord_squ <= radius_squ_) break;
            x_offset_++;
        }
    }

};


                // let delta = Point::new(*x, y) * 2 - self.center_2x;
                // (delta.length_squared() as u32) < self.threshold

}