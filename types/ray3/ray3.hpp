#pragma once

#include "types/vector3/Vector3.hpp"

namespace ymd{

template<typename T>
struct Ray3_t{
    Vector3_t<T> start;
    Vector3_t<T> direction;

    Ray3_t(){;}

    Ray3_t(const Ray3_t<T> & other):

        start(other.start),
        direction(other.direction){;}

    static constexpr Ray3_t from_start_and_dir(const Vector3_t<T> & _start, const Vector3_t<T> & _direction){
        Ray3_t ret;
        ret.start = _start;
        ret.direction = _direction.normalized();

        return ret;
    }

    static constexpr Ray3_t from_start_and_stop(const Vector3_t<T> & _start, const Vector3_t<T> & _stop){
        Ray3_t ret;
        ret.start = _start;
        ret.direction = (_stop - _start).normalized();
        return ret;
    }
};

template<typename T>
Ray3_t() -> Ray3_t<T>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Ray3_t<auto> & value){
    return os << os.brackets<'('>() << value.start << os.splitter() << value.direction << os.brackets<')'>();
}

}