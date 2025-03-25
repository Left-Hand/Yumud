#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"
#include "types/ray3/ray3.hpp"

using RGB = ymd::RGB_t<real_t>;

template<typename T>
struct TriangleSurface_t
{
    Vector3_t<T> v0, v1, v2;
    Vector3_t<T> E1, E2, normal;
};



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