#pragma once

#include "core/math/real.hpp"
#include "core/utils/option.hpp"
#include "prelude.hpp"

namespace ymd::robots{

enum class KinematicPairs{
    Planar,
    Revolute,
    Spherical,
    Prismatic,
    Cylindrical,
    // Screw,
    // Prismatic
};



template<arithmetic T, int Dimensions, KinematicPairs ... Args>
requires (Dimensions == 1 or Dimensions == 2 or Dimensions == 3)
struct Kinematics{};

template<arithmetic T>
struct DhParameters{
    T i;
    T di;
    T ai;
    // T i;
};


}