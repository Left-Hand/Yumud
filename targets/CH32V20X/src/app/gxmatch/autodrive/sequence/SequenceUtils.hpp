#pragma once

#include "types/vector3/Vector3_t.hpp"
#include "types/vector2/Vector2_t.hpp"
#include "types/ray2d/Ray2D_t.hpp"

namespace gxm{

using Ray = ymd::Ray2D_t<real_t>;
using Rays = std::vector<Ray>;
using Vector2 = ymd::Vector2_t<real_t>;
using Vector3 = ymd::Vector3_t<real_t>;

struct SequenceLimits{
    real_t max_gyro;
    real_t max_spd;
    real_t max_acc;
};


struct SequenceParas{
    size_t freq;
};

// namespace functions{

void linear(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
void fillet(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
void circle(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
// };
}