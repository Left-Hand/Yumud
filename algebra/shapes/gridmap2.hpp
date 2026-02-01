#pragma once

#include "prelude.hpp"


namespace ymd{

template<typename T>
struct GridMap2{
    math::Rect2<T> top_left_cell;
    math::Vec2<T> padding;
    math::Vec2<uint8_t> count;

    constexpr auto bounding_box() const {
        const auto top_left = top_left_cell.top_left;
        const auto size = math::Vec2<T>{
                static_cast<T>(top_left_cell.size.x * (count.x)), 
                static_cast<T>(top_left_cell.size.y * (count.y))} 
            + math::Vec2<T>{
                static_cast<T>(padding.x * (count.x - 1)), 
                static_cast<T>(padding.y * (count.y - 1))};
        return math::Rect2<T>(top_left, size);
    }

    constexpr bool contains_point(const math::Vec2<T> point) const{
        return contains_impl<0>(point.x) and contains_impl<1>(point.y);
    }

    constexpr bool contains_x(const T point_x) const{
        return contains_impl<0>(point_x);
    }
    constexpr bool contains_y(const T point_y) const{
        return contains_impl<1>(point_y);
    }

private:
    template<size_t I>
    constexpr bool contains_impl(const auto p) const {
        const auto offset = p - std::get<I>(top_left_cell.top_left);
        const auto cell_size = std::get<I>(top_left_cell.size);
        const auto gird_offset = [&]{
            const auto cell_with_padding_size = std::get<I>(top_left_cell.size) + std::get<I>(padding);
            return offset % cell_with_padding_size;
        }();

        return (gird_offset < cell_size);
    }
};

template<typename T>
struct is_placed_t<GridMap2<T>>:std::true_type{;};

// RenderIterator 特化
template<std::integral T>
struct RenderIterator<GridMap2<T>> {
    using Shape = GridMap2<T>;
    constexpr RenderIterator(const Shape & shape)
        : shape_(shape),
            y_stop_(shape_.bounding_box().y_range().stop),
            y_(shape.top_left_cell.y())
        {}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return y_ < y_stop_;
    }

    // 推进到下一行
    constexpr void seek_next() {
        y_++;
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename Color>
    // template<typename Color>
    Result<void, typename Target::Error> draw_filled(Target & target, const Color& color) {
        if(not shape_.contains_y(y_)) return Ok();
        T x = shape_.top_left_cell.x();
        // const auto stop_x = .stop;
        const auto count = shape_.count;

        for(size_t i = 0; i < count.x; i++){
            const T next_x = x + shape_.top_left_cell.w();
            const auto x_range = math::Range2u16::from_start_and_stop_unchecked(x, next_x);
            x = next_x + shape_.padding.x;

            if(const auto res = target.fill_x_range(x_range, color_cast<RGB565>(color));
                res.is_err()) return res;
            // target.fill_x_range(x_range, static_cast<RGB565>(ColorEnum::PINK)).examine();
            // target.fill_x_range(x_range, static_cast<RGB565>(ColorEnum::PINK));
            continue;
        }
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        return draw_filled(target, color);
    }


private:
    Shape shape_;
    T y_stop_;
    T y_;
};

}