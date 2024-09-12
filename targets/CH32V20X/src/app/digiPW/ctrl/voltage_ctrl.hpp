#pragma once

#include "current_ctrl.hpp"

class VoltageController{
protected:
    real_t targ_curr = 0;
    real_t curr_limit = 4;
public:
    real_t kp = real_t(0.00282);

    real_t update(const real_t targ_volt, const real_t real_volt, const real_t real_current);

    void reset(){
        targ_curr = 0;
    }
};