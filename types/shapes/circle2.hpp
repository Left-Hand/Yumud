#pragma once

#include "types/shapes/prelude.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/Rect2.hpp"

namespace ymd{

template<arithmetic T>
struct Dot{
	T radius;

	__fast_inline constexpr bool has_point(const Vec2<T> & p) const {
		return 	(p).length_squared() < square(radius);
	}
};

template<typename T>
struct is_placed_t<Dot<T>>: std::false_type{};

template<arithmetic T>
struct Circle2{
	Vec2<T> center;
	T radius;

	//判断点是否在圆内
	__fast_inline constexpr bool contains_point(const Vec2<T> & p) const {
		return 	(p - center).length_squared() < square(radius);
	}


	
	//判断是否与另一个圆相交
	__fast_inline constexpr bool intersects(const Circle2<T> & other) const {
		return (center - other.center).length_squared() <= square(radius + other.radius);
	}

	//判断是否与另一个圆相切
	__fast_inline constexpr bool tangent(const Circle2<T> & other) const {
		return (center - other.center).length_squared() == square(radius + other.radius);		
	}

	__fast_inline constexpr Rect2<T> bounding_box() const {
		return Rect2<T>::from_center_and_halfsize(
			center, Vec2<T>(radius, radius)
		);
	}

};


template<arithmetic T>
struct HorizonOval2{
	Vec2<T> left_center;
	T radius;
	T length;

	__fast_inline constexpr Rect2<T> bounding_box() const {
		const auto top_left = left_center + Vec2<T>(-radius, -radius);
		const auto size = Vec2<T>(radius * 2 + length, radius * 2);
		return Rect2<T>{top_left, size};
	}
};

template<typename T>
struct is_placed_t<Circle2<T>> : std::true_type {};

template<typename T>
struct is_placed_t<HorizonOval2<T>> : std::true_type {};

template<typename T>
__inline OutputStream & operator <<(OutputStream & os, const Circle2<T> & circle){
    return os << os.brackets<'('>() 
		<< circle.center << os.splitter()  
		<< circle.radius << os.brackets<')'>();
}


template<typename T>
__inline OutputStream & operator <<(OutputStream & os, const HorizonOval2<T> & oval){
    return os << os.brackets<'('>() 
	<< oval.left_center << os.splitter()  
	<< oval.radius << os.splitter()  
	<< oval.length << os.brackets<')'>();
}

}