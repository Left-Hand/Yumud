#pragma once

#include "algebra/shapes/prelude.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/regions/Rect2.hpp"

namespace ymd{

template<arithmetic T>
struct [[nodiscard]] Dot{
    using Self = Dot;
	T radius;

	__fast_inline constexpr bool has_point(const Vec2<T> & p) const {
		return 	(p).length_squared() < square(radius);
	}

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("radius")(self.radius)
        ;
    }
};
template<typename T>
struct [[nodiscard]] is_placed_t<Dot<T>>: std::false_type{};

}