#pragma once

#include "core/math/real.hpp"
#include "primitive/arithmetic/rescaler.hpp"

namespace ymd::hal{


template<typename Inst, typename T>
struct ScaledAnalogInput{
    constexpr ScaledAnalogInput(
        Inst & inst, 
        const Rescaler<T> & rescaler
    ):
        inst_(inst), rescaler_(rescaler){}

    __inline T get_value(){
        return rescaler_(inst_.get_voltage());
    }
private:
    Inst & inst_;
    Rescaler<T> rescaler_;
};

template<typename Inst, typename T>
ScaledAnalogInput(const Inst &, const Rescaler<T>) -> ScaledAnalogInput<Inst, T>;
};