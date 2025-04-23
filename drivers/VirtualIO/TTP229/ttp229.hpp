#pragma once

#include "core/platform.hpp"
#include "core/utils/Option.hpp"

namespace ymd::hal{
    class GpioIntf;
}

namespace ymd::drivers{
class TTP229{
protected:
    Option<uint8_t> num_ = None;
    uint16_t map_ = 0;
    hal::GpioIntf & sck_gpio_;
    hal::GpioIntf & sdo_gpio_;

public:
    TTP229(hal::GpioIntf & sck_gpio, hal::GpioIntf & sdo_gpio)
        :sck_gpio_(sck_gpio), sdo_gpio_(sdo_gpio){;}
    ~TTP229(){;}

    void update();
    bool is_idle();
    Option<uint8_t> get_num(){return num_;}
};

};