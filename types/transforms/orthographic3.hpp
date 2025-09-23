#pragma once

#include "matrix/static_matrix.hpp"

namespace ymd{

template<typename T>
class Orthographic3{
public:
    Matrix4x4<T> matrix;
};

}