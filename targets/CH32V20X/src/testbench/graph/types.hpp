#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"

template<typename T>
struct triangle_t
{
    Vector3_t<T> bbmin, bbmax;
    Vector3_t<T> v0, v1, v2;
    Vector3_t<T> E1, E2, normal;
};

template<typename T>
struct intersection_t
{
    int8_t i;
    T t;
};

template<typename T>
struct ray_t
{
    Vector3_t<T> start;
    Vector3_t<T> direction;
    Vector3_t<T> inv_direction;
};


template<typename T>
struct interaction_t: public intersection_t<T>{
    const struct triangle_t<T>* surface;
    Vector3_t<T> position;
    Vector3_t<T> normal;
};


template<typename T>
struct mat4_t{
    T m[4][4];
};