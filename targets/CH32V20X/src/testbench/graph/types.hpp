#pragma once

#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"

struct triangle_t
{
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

    ray_t(){;}

    ray_t(const ray_t<T> & other):

        start(other.start),
        direction(other.direction){;}

    static constexpr ray_t from_start_and_dir(const Vector3_t<T> & _start, const Vector3_t<T> & _direction){
        ray_t ret;
        ret.start = _start;
        ret.direction = _direction.normalized();

        return ret;
    }

    static constexpr ray_t from_start_and_stop(const Vector3_t<T> & _start, const Vector3_t<T> & _stop){
        ray_t ret;
        ret.start = _start;
        ret.direction = (_stop - _start).normalized();
        return ret;
    }
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
struct mat4_t{
private:
    using Row = std::array<T, 4>;
    using Buf = std::array<Row,4>;
public:
    constexpr mat4_t(const Row & r1, const Row & r2, const Row & r3, const Row & r4):
        buf_({r1, r2, r3, r4}){;}
    
    constexpr Row & operator[](const size_t idx){
        return buf_[idx];
    }

    constexpr const Row & operator[](const size_t idx) const {
        return buf_[idx];
    }

    constexpr const Vector3_t<real_t> & vx() const{
        return *reinterpret_cast<const Vector3_t<real_t> *>(&buf_[0]);
    }

    constexpr const Vector3_t<real_t> & vy() const{
        return *reinterpret_cast<const Vector3_t<real_t> *>(&buf_[1]);
    }

    constexpr const Vector3_t<real_t> & vz() const{
        return *reinterpret_cast<const Vector3_t<real_t> *>(&buf_[2]);
    }
private:
    Buf buf_;
};