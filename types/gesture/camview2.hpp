#pragma once

#include "types/gesture/isometry2.hpp"

namespace ymd{
template<typename T>
struct CamView2{
    Isometry2<T> pose;
    T zoom;
};

}