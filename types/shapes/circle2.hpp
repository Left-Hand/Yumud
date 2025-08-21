#pragma once

#include "types/shapes/prelude.hpp"
#include "types/vectors/vector2.hpp"

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
	Vec2<T> position;
	T radius;

	//判断点是否在圆内
	__fast_inline constexpr bool has_point(const Vec2<T> & p) const {
		return 	(p - position).length_squared() < square(radius);
	}
	
	//判断是否与另一个圆相交
	__fast_inline constexpr bool intersects(const Circle2<T> & other) const {
		return (position - other.position).length_squared() <= square(radius + other.radius);
	}

	//判断是否与另一个圆相切
	__fast_inline constexpr bool tangent(const Circle2<T> & other) const {
		return (position - other.position).length_squared() == square(radius + other.radius);		
	}

};

template<typename T>
struct is_placed_t<Circle2<T>> : std::true_type {};

template<typename T>
__inline OutputStream & operator <<(OutputStream & os, const Circle2<T> & circle){
    return os << '(' << circle.position << os.splitter()  << circle.radius << ')';
}

}