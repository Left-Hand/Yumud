#pragma once

#include "geometry/GeometryUtils.hpp"

namespace gxm{

using Ray = ymd::Ray2D_t<real_t>;
using Rays = std::vector<Ray>;
using Vector2 = ymd::Vector2_t<real_t>;
using Vector3 = ymd::Vector3_t<real_t>;

struct SequenceLimits{
    real_t max_gyro;
    real_t max_angular;
    real_t max_spd;
    real_t max_acc;
};


struct SequenceParas{
    size_t freq;
};

}