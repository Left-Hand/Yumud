#pragma once

#include <variant>
#include <type_traits>

#include "core/system.hpp"
#include "hal/timer/pwm/gpio_or_pwm.hpp"

namespace ymd::hal{
    class GpioIntf;
    class PwmIntf;
}

namespace ymd::drivers{

class LedIntf{
public:
    virtual void toggle() = 0;
    virtual void set_dutycycle(const real_t duty) = 0;
};

class LedGpio:public LedIntf{
protected:
    hal::GpioIntf & inst_;
    bool state = false;
    bool inversed;

public:
    LedGpio(hal::GpioIntf & inst, const bool inv = false):inst_(inst), inversed(inv){;}

    void toggle() override;

    void set_dutycycle(const real_t duty);

    explicit operator bool() const{return state ^ inversed;}
    explicit operator real_t() const{return state ^ inversed;}
};

class LedAnalog : public LedIntf{
protected:
    hal::GpioOrPwm inst;
    real_t last_duty_ = 0;
    bool inversed = false;

public:

    LedAnalog(auto & _inst, const bool inv = false):inst(_inst), inversed(inv){;}

    void toggle() override {
        last_duty_ = 1 - last_duty_;
        this->set_dutycycle(last_duty_);
    }

    void set_dutycycle(const real_t duty){
        last_duty_ = inversed ? 1 - duty : duty;
        inst.set_dutycycle(last_duty_);
    }
};

};