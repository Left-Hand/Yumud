#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"

struct triangle_t
{
    Vector3_t<float> bbmin, bbmax;
    Vector3_t<float> v0, v1, v2;
    Vector3_t<float> E1, E2, normal;
};

struct intersection_t
{
    int8_t i;
    float t;
};
static struct intersection_t intersection;

struct ray_t
{
    Vector3_t<float> start;
    Vector3_t<float> direction;
    Vector3_t<float> inv_direction;
};
static struct ray_t ray;

struct interaction_t
{
    int8_t i;
    float t;
    const struct triangle_t* surface;
    Vector3_t<float> position;
    Vector3_t<float> normal;
};



struct mat4_t
{
    float m[4][4];
};