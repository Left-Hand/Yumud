#pragma once

#include "core/math/real.hpp"
#include "primitive/rescaler.hpp"

namespace ymd::hal{


class AnalogInIntf{
public:
    virtual real_t get_voltage() = 0;
    virtual ~ AnalogInIntf() = default;
};

class AnalogOutIntf{
protected:
    virtual void write(const uint32_t data) = 0;
public:
    virtual ~AnalogOutIntf() = default;

    void set_voltage(const real_t volt){
    }
};

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