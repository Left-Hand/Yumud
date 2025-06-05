#pragma once

#include "../GateDriverIntf.hpp"

#define DRV8313_DEBUG

#ifdef DRV8313_DEBUG
#undef DRV8313_DEBUG
#define DRV8313_DEBUG(...) DEBUG_LOG(__VA_ARGS__)
#else
#define DRV8313_DEBUG(...)
#endif

namespace ymd::drivers{

class DRV8313:public GateDriver3{
protected:
    // using PWM3 = std::array<PwmIntf &, 3>;
    using EnablePort = VGpioPort<3>;
    using OptionalGpio = GpioIntf *;

    // PWM3 pwms_;
    EnablePort en_gpios_;
    OptionalGpio slp_gpio_;
    OptionalGpio fault_gpio_;
public:
    DRV8313(EnablePort && _en_port, OptionalGpio&& _slp_gpio, OptionalGpio&& _fault_gpio):
        // pwms_(std::move(_pwms)),
        en_gpios_(std::move(_en_port)),
        slp_gpio_(std::move(_slp_gpio)),
        fault_gpio_(std::move(_fault_gpio))
    {}

    void init(){
        DRV8313_DEBUG("DRV8313 init");

        if(slp_gpio_) (*slp_gpio_).outpp(false);
        if(fault_gpio_) (*fault_gpio_).inflt();
        for(auto & gpio : en_gpios_){
            if(gpio.isValid()) gpio.outpp(false);
        }
    }

    void sleep(const Enable en = EN){
        if(slp_gpio_) (*slp_gpio_).write(en);
    }

    void enable(const Enable en = EN){
        // for(auto & gpio : en_gpios_){
        //     if(gpio.isValid()) gpio = en;
        // }
    }

    DRV8313 & operator=(const UVW_Duty & duty) override{
        // auto [u, v, w] = duty;

        // pwms_[0] = u;
        // pwms_[1] = v;
        // pwms_[2] = w;

        return *this;
    }
};

using AT8325 = DRV8313;

};