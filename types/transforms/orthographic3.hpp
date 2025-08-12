#pragma once

#include "matrix/matrix_static.hpp"

namespace ymd{

template<typename T>
class Orthographic3{
public:
    Matrix4x4<T> matrix;
};

}