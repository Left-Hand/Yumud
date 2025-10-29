#pragma once

#include "core/platform.hpp"
#include "core/utils/Option.hpp"

namespace ymd::hal{
    class GpioIntf;
}

namespace ymd::drivers{


class TTP229{
public:
    explicit TTP229(
        Some<hal::GpioIntf *> sck_gpio, 
        Some<hal::GpioIntf *> sdo_gpio)
    :
        sck_gpio_(sck_gpio.deref()), 
        sdo_gpio_(sdo_gpio.deref()){;}

    ~TTP229(){;}

    void update();
    [[nodiscard]] bool is_idle();
    Option<uint8_t> get_num(){return num_;}
private:
    Option<uint8_t> num_ = None;
    uint16_t map_ = 0;
    hal::GpioIntf & sck_gpio_;
    hal::GpioIntf & sdo_gpio_;
};

};