#pragma once

#include "core/container/inline_vector.hpp"

#include "types/shapes/prelude.hpp"
#include "types/vectors/vector2.hpp"
#include "types/regions/Rect2.hpp"



namespace ymd{



template<arithmetic T>
struct [[nodiscard]] Circle2{
	using Self = Circle2;

	Vec2<T> center;
	T radius;

	//判断点是否在圆内
	[[nodiscard]] __fast_inline constexpr 
	bool contains_point(const Vec2<T> & p) const {
		return 	(p - center).length_squared() < square(radius);
	}
	
	//判断是否与另一个圆相交
	[[nodiscard]] __fast_inline constexpr 
	bool intersects(const Circle2<T> & other) const {
		return (center - other.center).length_squared() <= square(radius + other.radius);
	}

	//判断是否与另一个圆相切
	[[nodiscard]] __fast_inline constexpr 
	bool tangent(const Circle2<T> & other) const {
		return (center - other.center).length_squared() == square(radius + other.radius);		
	}

	[[nodiscard]] __fast_inline constexpr 
	Rect2<T> bounding_box() const {
		return Rect2<T>::from_center_and_halfsize(
			center, Vec2<T>(radius, radius)
		);
	}


    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("center")(self.center) << os.splitter()
            << os.field("radius")(self.radius)
        ;
    }

};




template<typename T>
struct is_placed_t<Circle2<T>> : std::true_type {};



}