#pragma once

#include "core/math/real.hpp"
#include "core/utils/rescaler.hpp"

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

    AnalogOutIntf & operator = (const real_t value){
        write(uni_to_u16(value));
        return *this;
    }

    void set_voltage(const real_t volt){
        *this = volt;
    }
};

template<typename Inst, typename T>
struct ScaledAnalogInput{
    constexpr ScaledAnalogInput(const Inst & inst, const Rescaler<T> & rescaler):
        inst_(inst), rescaler_(rescaler){}

    constexpr T get_value(){
        return rescaler_(inst_.get_voltage());
    }
private:
    const Inst & inst_;
    const Rescaler<T> & rescaler_;
};

template<typename Inst, typename T>
ScaledAnalogInput(const Inst &, const Rescaler<T>) -> ScaledAnalogInput<Inst, T>;
};