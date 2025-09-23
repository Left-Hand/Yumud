#pragma once

#include "core/utils/Option.hpp"

namespace ymd::drivers{

template<typename U_Inst, typename V_Inst, typename W_Inst>
struct UvwPwmgen{
    explicit UvwPwmgen(Some<U_Inst *> u_inst, Some<V_Inst *> v_inst, Some<W_Inst *> w_inst):
        pwm_insts_(std::make_tuple(u_inst.get(), v_inst.get(), w_inst.get())){;}

    template<typename UvwDutyCycle>
    __inline void set_dutycycle(const UvwDutyCycle dutycycle){
        std::get<0>(pwm_insts_)->set_dutycycle((dutycycle[0]));
        std::get<1>(pwm_insts_)->set_dutycycle((dutycycle[1]));
        std::get<2>(pwm_insts_)->set_dutycycle((dutycycle[2]));
    }

private:
    std::tuple<U_Inst *, V_Inst *, W_Inst *> pwm_insts_;
};


//CTAD for UvwPwmGen
template<typename U_Inst, typename V_Inst, typename W_Inst>
UvwPwmgen(U_Inst *, V_Inst *, W_Inst *) -> UvwPwmgen<U_Inst, V_Inst, W_Inst>;

}