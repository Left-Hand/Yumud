#pragma once

#include "sys/math/real.hpp"

class PowerController{
protected:
    real_t targ_current = 0;
    real_t current_limit = 4;
public:
    real_t kp = real_t(0.00282);

    // PowerController(CurrentController & _curr_ctrl):curr_ctrl(_curr_ctrl){;}
    real_t update(const real_t targ_power, const real_t real_power, const real_t real_current);
};