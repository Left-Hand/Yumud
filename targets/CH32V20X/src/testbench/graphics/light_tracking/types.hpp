#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vectors/vector3.hpp"
#include "types/regions/ray3.hpp"

using RGB = ymd::RGB_t<real_t>;
using namespace ymd;

template<typename T>
struct TriangleSurface_t{
    using Vec = Vector3<T>;

    Vector3<T> v0, v1, v2;
    // Vector3<T> normal;

    template<typename U>
    constexpr TriangleSurface_t(const TriangleSurface_t<U> & other):
        v0(static_cast<Vec>(other.v0)), 
        v1(static_cast<Vec>(other.v1)), 
        v2(static_cast<Vec>(other.v2))
        // , normal(static_cast<Vec>(other.normal))
        
        {;}

    constexpr TriangleSurface_t(
        const Vector3<auto> _v0,
        const Vector3<auto> _v1,
        const Vector3<auto> _v2
        // const Vector3<auto> _normal
    ):
        v0(static_cast<Vec>(_v0)), 
        v1(static_cast<Vec>(_v1)), 
        v2(static_cast<Vec>(_v2))
        // , normal(static_cast<Vec>(_normal)) 
        {;}
};

// TriangleSurface_t<real_t>

template<typename T>
struct CacheOf;

template<typename T>
struct CacheOf<TriangleSurface_t<T>> : public TriangleSurface_t<T>{
    Vector3<T> normal;

    template<typename U>
    constexpr CacheOf<TriangleSurface_t<T>> (const TriangleSurface_t<U> & other):
        TriangleSurface_t<T>(other),
        normal(calc_normal_from_points(
            static_cast<Vector3<T>>(other.v0), 
            static_cast<Vector3<T>>(other.v1), 
            static_cast<Vector3<T>>(other.v2)
        )){;}

private:
    static constexpr Vector3<T> calc_normal_from_points(
        const Vector3<T> & _v0,
        const Vector3<T> & _v1,
        const Vector3<T> & _v2
    ){
        return (_v1 - _v0).cross(_v2 - _v0).normalized();
    }
};

template<typename T>
using TriangleSurfaceCache_t = CacheOf<TriangleSurface_t<T>>;

template<typename T>
struct Intersection_t{
    int i;
    T t;
};

template<typename T>
struct Interaction_t{
    int i;
    T t;
    const TriangleSurface_t<T> & surface;
    Vector3<T> position;
    Vector3<T> normal;
};