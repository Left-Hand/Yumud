#pragma once

#include "../GateDriverIntf.hpp"
#include "types/regions/range2.hpp"
#include "hal/gpio/gpio_intf.hpp"
#include <optional>

namespace ymd::hal{
    struct TimerOC;
    struct TimerOCN;
    struct AdvancedTimer;
}

namespace ymd::drivers{

class MP1907:public GateDriver2Intf{
protected:
    hal::TimerOC & inst_;
    hal::TimerOCN & inst_n_;
    hal::GpioIntf & en_gpio_;
    Range2<real_t> duty_range_ = {real_t(0.03), real_t(0.97)};

public:
    MP1907(
        hal::TimerOC & ch, 
        hal::TimerOCN & chn, 
        hal::GpioIntf & en_gpio
    ):
        inst_(ch),
        inst_n_(chn),
        en_gpio_(en_gpio){;}

    void init();
    void setDutyRange(const Range2<real_t> & range){duty_range_ = range;}

    void enable(const Enable en = EN){
        en_gpio_.write((en == EN) ? HIGH : LOW);
    }
    MP1907 & operator=(const real_t duty);
};

};