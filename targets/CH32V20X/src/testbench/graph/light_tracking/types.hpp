#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"
#include "types/ray3/ray3.hpp"

using RGB = ymd::RGB_t<real_t>;
using namespace ymd;

template<typename T>
struct TriangleSurface_t{
    using Vec = Vector3_t<T>;

    Vector3_t<T> v0, v1, v2;
    // Vector3_t<T> normal;

    template<typename U>
    constexpr TriangleSurface_t(const TriangleSurface_t<U> & other):
        v0(static_cast<Vec>(other.v0)), 
        v1(static_cast<Vec>(other.v1)), 
        v2(static_cast<Vec>(other.v2))
        // , normal(static_cast<Vec>(other.normal))
        
        {;}

    constexpr TriangleSurface_t(
        const Vector3_t<auto> _v0,
        const Vector3_t<auto> _v1,
        const Vector3_t<auto> _v2
        // const Vector3_t<auto> _normal
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
    Vector3_t<T> normal;

    template<typename U>
    constexpr CacheOf<TriangleSurface_t<T>> (const TriangleSurface_t<U> & other):
        TriangleSurface_t<T>(other),
        normal(calc_normal_from_points(
            static_cast<Vector3_t<T>>(other.v0), 
            static_cast<Vector3_t<T>>(other.v1), 
            static_cast<Vector3_t<T>>(other.v2)
        )){;}

private:
    static constexpr Vector3_t<T> calc_normal_from_points(
        const Vector3_t<T> & _v0,
        const Vector3_t<T> & _v1,
        const Vector3_t<T> & _v2
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
    Vector3_t<T> position;
    Vector3_t<T> normal;
};