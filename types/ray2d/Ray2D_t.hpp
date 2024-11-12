#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"

namespace yumud{

template<arithmetic T>
struct Ray2D_t{
public:
    Vector2_t<T> p;
    T a;

public:
    constexpr Ray2D_t(){;}

    constexpr Ray2D_t(const Vector2_t<auto> & _from, const Vector2_t<auto> & _to): 
            p(static_cast<Vector2_t<T>>(_from)), a((_to - _from).angle()){;}

    template<arithmetic U = T>
    constexpr Ray2D_t(const std::tuple<U, U, U> & tup) : 
            p((Vector2_t<T>(std::get<0>(tup), std::get<1>(tup)))),
            a(std::get<3>(tup)){;}

	__fast_inline constexpr bool operator==(const Ray2D_t & other) const{
        return is_equal_approx(this->p == other.p) and is_equal_appro(this->a, other.a);
    }

	__fast_inline constexpr bool operator!=(const Ray2D_t & other) const{
        return (*this == other) == false; 
    }


    __fast_inline constexpr T distance_to(const Vector2_t<T> & pt) const{
        //TODO
        
        return 0;
    }

    __fast_inline constexpr bool has_point(const Vector2_t<T> & pt) const{
        return is_equal_approx(distance_to(pt), 0);
    }

    __fast_inline constexpr bool parrel_with(const Ray2D_t & other){
        return is_equal_approx(this->a, other.a);
    }

    __fast_inline constexpr std::optional<Vector2_t<T>> intersection(const Ray2D_t<T> & other) const{
        if(this->parrel_with(other)) return std::nullopt;

        //TODO
        return {0,0};
    }
};

}
