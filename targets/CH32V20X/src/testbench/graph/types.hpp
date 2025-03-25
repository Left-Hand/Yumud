#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"

struct triangle_t
{
    Vector3_t<real_t> bbmin, bbmax;
    Vector3_t<real_t> v0, v1, v2;
    Vector3_t<real_t> E1, E2, normal;
};


template<typename T>
struct ray_t{
    Vector3_t<T> start;
    Vector3_t<T> direction;
    Vector3_t<T> inv_direction;

    ray_t(){;}

    // ray_t(const ray_t<auto> & other):

    //     start(other.start),
    //     direction(other.direction),
    //     inv_direction(other.inv_direction)
    // {}

    ray_t(const ray_t<T> & other):

        start(other.start),
        direction(other.direction),
        inv_direction(other.inv_direction)
    {}

    ray_t(const Vector3_t<T> & _start, const Vector3_t<T> & _direction): 
        start(_start),
        direction(_direction),
        inv_direction(Vector3_t<T>::from_rcp(_direction))
    {}
};

template<typename T>
struct intersection_t
{
    int8_t i;
    T t;

    intersection_t(){}
    intersection_t(const intersection_t<T> & other) = default;
    // intersection_t(const intersection_t<auto> & other):
    //     i(other.i),
    //     t(static_cast<T>(other.t)){}

};

template<typename T>
struct interaction_t: public intersection_t<T>{
    const struct triangle_t * surface;
    Vector3_t<T> position;
    Vector3_t<T> normal;
};


template<typename T>
struct mat4_t{
    T m[4][4];
};