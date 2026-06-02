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


};