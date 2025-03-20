#pragma once

#include "core/platform.h"
#include "types/Circle2D/Circle2D_t.hpp"
#include "types/Range/range.hpp"

namespace ymd{

template<arithmetic T>
struct Arc2D_t{
public:
	Vector2_t<T> org;
	T r;
	Range_t<T> range;
public:
	__fast_inline constexpr Arc2D_t(const Vector2_t<T> & _org, const T & _r, const Range_t<T> _range):
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