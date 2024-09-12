#pragma once

class PowerController{
protected:
    real_t targ_current = 0;
    real_t current_limit = 4;
public:
    real_t kp = real_t(0.00282);

    CurrentController & curr_ctrl;

    PowerController(CurrentController & _curr_ctrl):curr_ctrl(_curr_ctrl){;}
    void update(const real_t targ_power, const real_t real_power, const real_t real_current){
        auto error = targ_power - real_power;
        real_t kp_contribute = error * kp;

        real_t delta = kp_contribute;

        targ_current += delta;
        targ_current = CLAMP(targ_current, 0, current_limit);

        curr_ctrl.update(targ_current, real_current);
    }
};