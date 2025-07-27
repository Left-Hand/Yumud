#pragma once

#include "pose2.hpp"

namespace ymd{
template<typename T>
struct CamView2{
    Pose2<T> pose;
    T zoom;
};

}