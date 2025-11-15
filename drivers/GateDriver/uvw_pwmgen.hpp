#pragma once

#include "core/utils/Option.hpp"

namespace ymd::drivers{

template<typename U_Inst, typename V_Inst, typename W_Inst>
struct UvwPwmgen{
    explicit UvwPwmgen(Some<U_Inst *> u_gen, Some<V_Inst *> v_gen, Some<W_Inst *> w_gen):
        u_gen_(u_gen.deref()),
        v_gen_(v_gen.deref()),
        w_gen_(w_gen.deref())
    {}

    template<typename UvwDutyCycle>
    __inline void set_dutycycle(const UvwDutyCycle dutycycle){
        u_gen_.set_dutycycle((dutycycle.template get<0>()));
        v_gen_.set_dutycycle((dutycycle.template get<1>()));
        w_gen_.set_dutycycle((dutycycle.template get<2>()));
    }

private:
    U_Inst & u_gen_;
    V_Inst & v_gen_;
    W_Inst & w_gen_;
};


//CTAD for UvwPwmGen
template<typename U_Inst, typename V_Inst, typename W_Inst>
UvwPwmgen(U_Inst *, V_Inst *, W_Inst *) -> UvwPwmgen<U_Inst, V_Inst, W_Inst>;

}