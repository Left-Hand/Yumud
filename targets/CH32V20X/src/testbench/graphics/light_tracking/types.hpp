#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "algebra/vectors/vec3.hpp"
#include "algebra/regions/ray3.hpp"

using namespace ymd;

template<typename T>
struct TriangleSurface{
    using Vec = math::Vec3<T>;

    math::Vec3<T> v0, v1, v2;

    template<typename U>
    constexpr TriangleSurface(const TriangleSurface<U> & other):
        v0(static_cast<Vec>(other.v0)), 
        v1(static_cast<Vec>(other.v1)), 
        v2(static_cast<Vec>(other.v2))
        {;}

    constexpr TriangleSurface(
        const math::Vec3<auto> _v0,
        const math::Vec3<auto> _v1,
        const math::Vec3<auto> _v2
    ):
        v0(static_cast<math::Vec3<T>>(_v0)), 
        v1(static_cast<math::Vec3<T>>(_v1)), 
        v2(static_cast<math::Vec3<T>>(_v2))
        {;}
};


template<typename T>
struct CacheOf;

template<typename T>
struct CacheOf<TriangleSurface<T>> : public TriangleSurface<T>{
    math::Vec3<T> normal;

    template<typename U>
    constexpr CacheOf<TriangleSurface<T>> (const TriangleSurface<U> & other):
        TriangleSurface<T>(other),
        normal(calc_normal_from_points(
            static_cast<math::Vec3<T>>(other.v0), 
            static_cast<math::Vec3<T>>(other.v1), 
            static_cast<math::Vec3<T>>(other.v2)
        )){;}

private:
    static constexpr math::UnitVec3<T> calc_normal_from_points(
        const math::Vec3<T> & _v0,
        const math::Vec3<T> & _v1,
        const math::Vec3<T> & _v2
    ){
        return (_v1 - _v0).cross(_v2 - _v0).normalized();
    }
};

template<typename T>
using TriangleSurfaceCache = CacheOf<TriangleSurface<T>>;

template<typename T>
struct Intersection{
    int i;
    T t;
};

template<typename T>
struct Interaction{
    int i;
    T t;
    const TriangleSurface<T> & surface;
    math::Vec3<T> position;
    math::Vec3<T> normal;
};