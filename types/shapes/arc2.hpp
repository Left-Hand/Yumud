#pragma once

#include "core/platform.hpp"
#include "types/regions/range2.hpp"
#include "types/shapes/Circle2.hpp"

namespace ymd{

template<arithmetic T>
struct Arc2{
public:
	Vector2<T> center;
	T radius;
	Range2<T> angle_range;
public:
	__fast_inline constexpr Arc2(
		const Vector2<T> & _org, 
		const T & _r, 
		const Range2<T> _range
	):
        center(_org), radius(_r), angle_range(_range) {}
	
	__fast_inline constexpr bool intersects(const Arc2<T> & other) const {
		//FIXME

		return (center - other.center).length_squared() <= square(radius + other.radius);
	}

	__fast_inline constexpr bool tangent(const Arc2<T> & other) const {

		//FIXME
		return (center - other.center).length_squared() == square(radius + other.radius);		
	}
};


__inline OutputStream & operator <<(OutputStream & os, const Arc2<auto> & arc){
	const auto splt = os.splitter();
    return os << '(' << arc.center << splt << arc.radius << splt << arc.angle_range << ')';
}

}