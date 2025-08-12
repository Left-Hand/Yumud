#pragma once

#include "core/platform.hpp"
#include "types/vectors/vector2.hpp"

namespace ymd{

template<arithmetic T>
struct Circle2{
public:
	Vec2<T> org;
	T r;
public:
	__fast_inline constexpr Circle2(const Vec2<T> & _org, const T & _r):
        org(_org), r(_r){}

	__fast_inline constexpr bool contains(const Vec2<T> & p) const {
		return 	(p - org).length_squared() < r * r;
	}
	
	__fast_inline constexpr bool intersects(const Circle2<T> & other) const {
		return (org - other.org).length_squared() <= square(r + other.r);
	}

	__fast_inline constexpr bool tangent(const Circle2<T> & other) const {
		return (org - other.org).length_squared() == square(r + other.r);		
	}

};


__inline OutputStream & operator <<(OutputStream & os, const Circle2<auto> & circle){
    return os << '(' << circle.org << os.splitter()  << circle.r << ')';
}

}