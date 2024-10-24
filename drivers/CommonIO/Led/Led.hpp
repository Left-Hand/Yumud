#pragma once

#include "../../sys/core/system.hpp"

#include <variant>
#include <type_traits>

#include "../../hal/timer/pwm/gpio_or_pwm.hpp"


struct GpioConcept;
struct PwmChannel;

class LedConcept{
public:
    virtual void toggle() = 0;
    virtual LedConcept & operator = (const real_t duty) = 0;
};

class LedGpio:public LedConcept{
protected:
    GpioConcept & inst;
    bool state = false;
    bool inversed;

public:
    LedGpio(GpioConcept & _instance, const bool inv = false):inst(_instance), inversed(inv){;}

    LedGpio & operator = (const real_t duty) override;

    void toggle() override;

    operator bool() const{return state ^ inversed;}
    operator real_t() const{return state ^ inversed;}
};

class LedAnalog : public LedConcept{
protected:
    GpioOrPwm inst;
    real_t last_duty_ = 0;
    bool inversed = false;

public:

    LedAnalog(gpio_or_pwm auto & _inst, const bool inv = false):inst(_inst), inversed(inv){;}

    void toggle() override {
        last_duty_ = 1 - last_duty_;
        *this = (last_duty_);
    }

    LedAnalog & operator = (const real_t duty) override{
        last_duty_ = inversed ? 1 - duty : duty;
        inst = last_duty_;

        return *this;
    }
};
