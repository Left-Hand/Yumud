#pragma once

#include "types/shapes/prelude.hpp"

namespace ymd{


template<typename T>
struct RoundedRect2{
    Rect2<T> bounding_rect;
    T radius;

    constexpr auto bounding_box() const {
        return bounding_rect;
    }
};

template<typename T>
struct is_placed_t<RoundedRect2<T>> : std::true_type {};
template<typename T>
struct RoundedRect2SliceIterator{
public:
    constexpr RoundedRect2SliceIterator(const RoundedRect2<T> & shape):
        x_range_(shape.bounding_rect.x_range()),
        left_and_right_center_x_(shape.bounding_rect.x_range()
            .shrink(shape.radius).examine()),
        y_range_(shape.bounding_rect.y_range()),
        y_(y_range_.start),
        radius_(shape.radius),
        radius_squ_(square(shape.radius))
    {
        replace_x();
    }

    constexpr void advance(){
        y_++;
        replace_x();
    }

    constexpr bool has_next() const {
        return y_ < y_range_.stop;
    }

    constexpr Range2u16 x_range() const{
        if(get_y_overhit()){
            return Range2u16::from_start_and_stop_unchecked(
                left_and_right_center_x_.start  + x_offset_,
                left_and_right_center_x_.stop - x_offset_
            );
        }else{
            return x_range_;
        }
    }

    constexpr std::tuple<Range2u16, Range2u16> left_and_right() const {
        const auto [left, right] = x_range();

        return {Range2u16{left, left + 1}, Range2u16{right - 1, right}};
    }

private:
    Range2<uint16_t> x_range_;
    Range2<uint16_t> left_and_right_center_x_;
    Range2<uint16_t> y_range_;
    uint16_t y_;
    uint16_t radius_;
    uint32_t radius_squ_;

    int16_t x_offset_ = 0;

    constexpr void replace_x(){
        x_offset_ = -radius_;

        const uint32_t squ_y_offset = static_cast<uint32_t>(square(static_cast<uint32_t>(get_y_overhit())));
        for(x_offset_ = -radius_; x_offset_ <= 0; x_offset_++){
            if (static_cast<uint32_t>(square(static_cast<int32_t>(x_offset_))) + squ_y_offset <= radius_squ_)
                break;
        }
        // x_offset_ = -radius_;
    }

    constexpr bool is_y_at_edge() const {
        return y_ == (y_range_.start) || y_ == (y_range_.stop);
    }

    constexpr uint16_t get_y_overhit() const {
        if(y_ < y_range_.start + radius_){
            return y_range_.start + radius_ - y_;
        }else if(y_ > y_range_.stop - radius_){
            return y_ - (y_range_.stop - radius_);
        }else{
            return 0;
        }
        // return MIN(uint16_t(left_and_right_center_x_.start) - y_), 0) MN(y_ - (left_and_right_center_x_.stop)));
    }
};







template<std::integral T>
struct DrawDispatchIterator<RoundedRect2<T>> {
    using Shape = RoundedRect2<T>;
    using Iterator = RoundedRect2SliceIterator<T>;

    constexpr DrawDispatchIterator(const Shape & shape)
        : iter_(shape){;}

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
        const auto x_range = iter_.x_range();
        if(auto res = target.fill_x_range({x_range.start, x_range.stop}, color);
            res.is_err()) return res;
        
        return Ok();
    }

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
    Iterator iter_;
};
}