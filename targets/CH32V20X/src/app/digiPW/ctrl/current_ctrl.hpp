#pragma once

#include "sys/math/real.hpp"

struct CurrentController{
protected:
    real_t kp = real_t(0.057);
    real_t duty;
public:
    real_t update(const real_t targ_current, const real_t real_current);
};
