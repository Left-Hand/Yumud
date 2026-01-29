#pragma once

#include "algebra/vectors/vec3.hpp"
namespace ymd{

template<typename T>
struct Sphere{
    math::Vec3<T> org;
    T radius;

    friend OutputStream & operator << (OutputStream & os, const Sphere & self){
        return os << os.brackets<'{'>() << 
            self.org << os.splitter() << self.radius << 
            os.brackets<'}'>();
    }
};

}