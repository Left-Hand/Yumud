#pragma once

#include "types/ray2d/Ray2D_t.hpp"

namespace gxm{

using Ray = yumud::Ray2D_t<real_t>;
using Rays = std::vector<Ray>;

struct SequenceLimits{
    real_t max_gyro;
    real_t max_acc;
};


struct SequenceParas{
    size_t points;
    real_t duration;
};

// namespace functions{

void linear(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
void fillet(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
void circle(Rays & curve, const SequenceLimits & limits, const SequenceParas & paras, const Ray & a, const Ray & b);
// };
}