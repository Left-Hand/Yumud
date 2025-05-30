#pragma once

#include "core/platform.hpp"
#include "types/regions/range2/range2.hpp"
#include "types/shapes/Circle2/Circle2.hpp"

namespace ymd{

template<arithmetic T>
struct Arc2D_t{
public:
	Vector2<T> org;
	T r;
	Range2<T> range;
public:
	__fast_inline constexpr Arc2D_t(const Vector2<T> & _org, const T & _r, const Range2<T> _range):
        org(_org), r(_r), range(_range) {}
	
	__fast_inline constexpr bool intersects(const Arc2D_t<T> & other) const {
		//FIXME

		return (org - other.org).length_squared() <= square(r + other.r);
	}

	__fast_inline constexpr bool tangent(const Arc2D_t<T> & other) const {

		//FIXME
		return (org - other.org).length_squared() == square(r + other.r);		
	}
};


__inline OutputStream & operator <<(OutputStream & os, const Arc2D_t<auto> & arc){
	const auto splt = os.splitter();
    return os << '(' << arc.org << splt << arc.r << splt << arc.range << ')';
}

}