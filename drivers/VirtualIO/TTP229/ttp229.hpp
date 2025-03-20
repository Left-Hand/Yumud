#pragma once

#include "core/platform.hpp"

namespace ymd::hal{
    class GpioIntf;
}

namespace ymd::drivers{
class TTP229{
protected:
    int8_t num = -1;
    uint16_t map = 0;
    hal::GpioIntf & sck_gpio_;
    hal::GpioIntf & sdo_gpio_;

public:
    TTP229(hal::GpioIntf & sck_gpio, hal::GpioIntf & sdo_gpio)
        :sck_gpio_(sck_gpio), sdo_gpio_(sdo_gpio){;}
    ~TTP229(){;}

    void scan();
    bool hasKey(){return (bool)(num >= 0);}
    bool isIdle();
    int8_t getNum(){return num+1;}
    int8_t getONum(){return num;}
    void getMapData(uint16_t & _map){_map = map;}
};

};