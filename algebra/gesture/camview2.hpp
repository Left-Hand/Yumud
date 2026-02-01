#pragma once

#include "algebra/gesture/isometry2.hpp"

namespace ymd::math{
template<typename T>
struct [[nodiscard]] CamView2{
    Isometry2<T> pose;
    T zoom;
};

}