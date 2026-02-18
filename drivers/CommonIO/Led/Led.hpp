#pragma once

#include <variant>
#include <type_traits>

#include "core/system.hpp"
#include "core/math/real.hpp"

namespace ymd::hal{
    class GpioIntf;
    class PwmIntf;
}

namespace ymd::drivers{

class LedIntf{
public:
    virtual void toggle() = 0;
    virtual void set_dutycycle(const iq16 dutycycle) = 0;
};

class LedGpio final:public LedIntf{

public:
    LedGpio(hal::GpioIntf & inst, const bool inv = false):inst_(inst), inversed(inv){;}

    void toggle();

    void set_dutycycle(const iq16 dutycycle);
private:
    hal::GpioIntf & inst_;
    bool state = false;
    bool inversed;

};

#if 0
class LedAnalog : public LedIntf{
protected:
    hal::GpioOrPwm inst;
    iq16 last_duty_ = 0;
    bool inversed = false;

public:

    LedAnalog(auto & _inst, const bool inv = false):inst(_inst), inversed(inv){;}

    void toggle() {
        last_duty_ = 1 - last_duty_;
        this->set_dutycycle(last_duty_);
    }

    void set_dutycycle(const iq16 dutycycle{
        last_duty_ = inversed ? 1 - duty : duty;
        inst.set_dutycycle(last_duty_);
    }
};
#endif

};