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

struct RGB{
    real_t r, g, b;

    RGB & operator *= (const real_t val){
        r *= val;
        g *= val;
        b *= val;

        return *this;
    }

    RGB & operator *= (const RGB & val){
        r *= val.r;
        g *= val.g;
        b *= val.b;

        return *this;
    }

    RGB & operator /= (const real_t val){
        const auto inv_val = 1 / val;
        r *= inv_val;
        g *= inv_val;
        b *= inv_val;

        return *this;
    }

    RGB & operator /= (const RGB & val){
        r /= val.r;
        g /= val.g;
        b /= val.b;

        return *this;
    }

    RGB operator * (const real_t val) const {
        auto ret = RGB(*this);
        ret *= val;
        return ret;
    }

    RGB operator * (const RGB & val) const {
        auto ret = RGB(*this);
        ret *= val;
        return ret;
    }


    RGB operator / (const real_t val) const {
        auto ret = RGB(*this);
        ret /= val;
        return ret;
    }

    // RGB operator / (const RGB & val){
    //     auto ret = RGB(*this);
    //     ret /= val;
    //     return ret;
    // }

    RGB & operator += (const RGB & val){
        r += val.r;
        g += val.g;
        b += val.b;

        return *this;
    }

    RGB operator + (const RGB & val) const {
        auto ret = RGB(*this);
        ret += val;
        return ret;
    }

    RGB & operator -= (const RGB & val){
        r -= val.r;
        g -= val.g;
        b -= val.b;

        return *this;
    }

    RGB operator - (const RGB & val) const {
        auto ret = RGB(*this);
        ret -= val;
        return ret;
    }

    RGB operator - () const {
        return RGB(-r,-g,-b);
    }
};


template<typename T>
struct ray_t{
    Vector3_t<T> start;
    Vector3_t<T> direction;
    Vector3_t<T> inv_direction;

    ray_t(){;}

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
struct intersection_t{
    int i;
    T t;
};

template<typename T>
struct interaction_t{
    int i;
    T t;
    const struct triangle_t & surface;
    Vector3_t<T> position;
    Vector3_t<T> normal;
};


template<typename T>
using mat4_t = std::array<std::array<T,4>,4>;


// template<typename T>
// struct mat4_t{

// }