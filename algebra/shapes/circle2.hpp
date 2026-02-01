#pragma once

#include "core/container/heapless_vector.hpp"

#include "algebra/shapes/prelude.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/Rect2.hpp"



namespace ymd::math{

template<arithmetic T>
struct [[nodiscard]] Circle2{
	using Self = Circle2;

	math::Vec2<T> center;
	T radius;

	//判断点是否在圆内
	[[nodiscard]] constexpr 
	bool contains_point(const math::Vec2<T> & p) const {
		return 	(p - center).length_squared() < square(radius);
	}
	
	//判断是否与另一个圆相交
	[[nodiscard]] constexpr 
	bool intersects(const Circle2<T> & other) const {
		return (center - other.center).length_squared() <= square(radius + other.radius);
	}

	//判断是否与另一个圆相切
	[[nodiscard]] constexpr 
	bool tangent(const Circle2<T> & other) const {
		return (center - other.center).length_squared() == square(radius + other.radius);		
	}

	[[nodiscard]] constexpr 
	math::Rect2<T> bounding_box() const {
		return math::Rect2<T>::from_center_and_halfsize(
			center, math::Vec2<T>(radius, radius)
		);
	}


    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("center")(self.center) << os.splitter()
            << os.field("radius")(self.radius)
        ;
    }

};



}