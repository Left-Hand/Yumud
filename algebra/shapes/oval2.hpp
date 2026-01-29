#pragma once

#include "prelude.hpp"

namespace ymd{


    

template<arithmetic T>
struct HorizonOval2{
	Vec2<T> left_center;
	T radius;
	T length;

	static constexpr Option<HorizonOval2<T>> try_from_bounding_box(const Rect2<T> bb){
		const T h = bb.h();
		const T w = bb.w();
		if(w < h) return None;
		const T radius = static_cast<T>(h / 2);
		const T length = static_cast<T>(w - h);
		const auto left_center = bb.top_left + Vec2<T>(radius, radius);
		return Some(HorizonOval2<T>{
			.left_center = left_center,
			.radius = radius,
			.length = length
		});
	}
	__fast_inline constexpr Rect2<T> bounding_box() const {
		const auto top_left = left_center + Vec2<T>(-radius, -radius);
		const auto size = Vec2<T>(radius * 2 + length, radius * 2);
		return Rect2<T>{top_left, size};
	}
};

template<arithmetic T>
struct VerticalOval2{
	Vec2<T> top_center;
	T radius;
	T length;

	static constexpr Option<VerticalOval2<T>> from_bounding_box(const Rect2<T> bb){
		const T h = bb.h();
		const T w = bb.w();
		if(h < w) return None;
		const T radius = static_cast<T>(w / 2);
		const T length = static_cast<T>(h - w);
		const Vec2<T> top_center = bb.top_left + Vec2<T>{radius, radius};
		return Some(VerticalOval2<T>{
			.top_center = top_center,
			.radius = radius,
			.length = length
		});
	}
	__fast_inline constexpr Rect2<T> bounding_box() const {
		const auto top_left = top_center + Vec2<T>(-radius, -radius);
		const auto size = Vec2<T>(radius * 2, radius * 2 + length);
		return Rect2<T>{top_left, size};
	}
};

template<typename T>
struct is_placed_t<HorizonOval2<T>> : std::true_type {};

template<typename T>
struct is_placed_t<VerticalOval2<T>> : std::true_type {};



template<typename T>
__inline OutputStream & operator <<(OutputStream & os, const HorizonOval2<T> & oval){
    return os << os.brackets<'('>() 
	<< oval.left_center << os.splitter()  
	<< oval.radius << os.splitter()  
	<< oval.length << os.brackets<')'>();
}

template<typename T>
__inline OutputStream & operator <<(OutputStream & os, const VerticalOval2<T> & oval){
    return os << os.brackets<'('>() 
	<< oval.top_center << os.splitter()  
	<< oval.radius << os.splitter()  
	<< oval.length << os.brackets<')'>();
}


template<typename T>
struct VerticalOval2SliceIterator{
public:
    constexpr VerticalOval2SliceIterator(const VerticalOval2<T> & shape):
        x_range_(shape.bounding_box().x_range()),
        y_range_(shape.bounding_box().y_range()),
        y_(y_range_.start),
        x0_(shape.top_center.x),
        radius_(shape.radius),
        radius_squ_(math::square(shape.radius))
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
                static_cast<uint16_t>(x0_ + x_offset_),
                static_cast<uint16_t>(x0_ - x_offset_)
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
    Range2<uint16_t> y_range_;
    uint16_t y_;
    uint16_t x0_;
    uint16_t radius_;
    uint16_t radius_squ_;

    int16_t x_offset_ = 0;

    constexpr void replace_x(){

        const uint32_t squ_y_offset = static_cast<uint32_t>(
            math::square(static_cast<uint32_t>(get_y_overhit())));

        #if 1
        for(int32_t x_offset = -radius_; x_offset <= 0; x_offset++){
            if (static_cast<uint32_t>(math::square(x_offset)) + squ_y_offset <= radius_squ_){
                x_offset_ = static_cast<int16_t>(x_offset);
                return;
            }
        }
        __builtin_unreachable();
        #else
        // x_offset_ = static_cast<int16_t>(-ymd::sqrt(fixed_t<16>(radius_squ_ - squ_y_offset)));
        #endif
    }

    constexpr bool is_y_at_edge() const {
        return y_ == (y_range_.start) || y_ == (y_range_.stop);
    }

    static constexpr int32_t fast_relu_i32(const int32_t x){
        return x & (~(x >> 31));
    }
    constexpr uint16_t get_y_overhit() const {
        // const uint16_t y_top    = static_cast<uint16_t>(y_range_.start + radius_);
        // const uint16_t y_bottom = static_cast<uint16_t>(y_range_.stop - radius_);
        // const int32_t y_top = static_cast<int32_t>(y_range_.start) + static_cast<int32_t>(radius_);
        // const int32_t y_bottom = static_cast<int32_t>(y_range_.stop) - static_cast<int32_t>(radius_);

        // if(y_ < y_top){
        //     return y_top - y_;
        // }else if(y_ > y_bottom){
        //     return y_ - (y_bottom);
        // }else{
        //     return 0;
        // }

        const int32_t y_top_over = static_cast<int32_t>(y_range_.start + radius_ - y_);
        const int32_t y_bottom_over = static_cast<int32_t>(y_ - y_range_.stop + radius_);
        return MAX(y_top_over, y_bottom_over, 0);
        // return fast_relu_i32(y_top_over) + fast_relu_i32(y_bottom_over);
        // 计算上边界超出的距离（如果y_ < y_top则为正，否则为0）
        // const int32_t over_top = (y_top - y_) & ~(static_cast<int32_t>(y_ - y_top) >> 31);
        // const int32_t over_top = MAX(y_top - y_, 0);
        
        // 计算下边界超出的距离（如果y_ > y_bottom则为正，否则为0）
        // const int32_t over_bottom = (y_ - y_bottom) & ~(static_cast<int32_t>(y_bottom - y_) >> 31);
        // const int32_t over_bottom = MAX(y_ - y_bottom, 0);
        // 合并结果（两个条件互斥，所以可以直接相加）
        // return over_top + over_bottom;
    }
};



template<std::integral T>
struct RenderIterator<HorizonOval2<T>> {
    using Shape = HorizonOval2<T>;
    using Iterator = CircleBresenhamIterator<T>;

    constexpr RenderIterator(const Shape & shape)
        : iter_(Circle2<T>{.center = shape.left_center, .radius = shape.radius}),
            length_(shape.length){}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return iter_.has_next();
    }

    // 推进到下一行
    constexpr void seek_next() {
        iter_.advance();
    }

    // 绘制当前行的所有点
    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_filled(Target& target, const Color& color) {
        // 绘制当前行的范围
        const auto x_range = iter_.x_range();
        const uint16_t x_start = static_cast<uint16_t>(std::max<int32_t>(x_range.start, 0));
        if(auto res = target.fill_x_range({x_start, x_range.stop + length_}, color);
            res.is_err()) return res;
        
        return Ok();
    }

    template<DrawTargetConcept Target, typename Color>
    Result<void, typename Target::Error> draw_hollow(Target& target, const Color& color) {
        // 绘制当前行的范围
        if(iter_.is_y_at_edge()){
            const auto x_range = iter_.x_range();
            if(const auto res = target.fill_x_range({x_range.start, x_range.stop + length_}, color);
                res.is_err()) return res;
            return Ok();
        }else{
            const auto [left_x_range, right_x_range] = iter_.left_and_right();
            if(const auto res = target.fill_x_range(left_x_range, color);
                res.is_err()) return res;
            if(const auto res = target.fill_x_range(right_x_range.shift(length_), color);
                res.is_err()) return res;
            return Ok();
        }

    }

private:
    Iterator iter_;
    T length_;
};




template<std::integral T>
struct RenderIterator<VerticalOval2<T>> {
    using Shape = VerticalOval2<T>;
    using Iterator = VerticalOval2SliceIterator<T>;

    constexpr RenderIterator(const Shape & shape)
        : iter_(shape){;}

    // 检查是否还有下一行
    constexpr bool has_next() const {
        return iter_.has_next();
    }

    // 推进到下一行
    constexpr void seek_next() {
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