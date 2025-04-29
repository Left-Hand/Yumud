#pragma once

#include "../CoilDriver.hpp"
#include "types/range/range.hpp"
#include "hal/gpio/gpio_intf.hpp"
#include <optional>

namespace ymd::hal{
    struct TimerOC;
    struct TimerOCN;
    struct AdvancedTimer;
}

namespace ymd::drivers{

class MP1907:public Coil2DriverIntf{
protected:
    hal::TimerOC & inst_;
    hal::TimerOCN & inst_n_;
    hal::GpioIntf & en_gpio_;
    Range2_t<real_t> duty_range = {real_t(0.03), real_t(0.97)};

public:
    MP1907(hal::TimerOC & ch, hal::TimerOCN & chn, hal::GpioIntf & en_gpio):
        inst_(ch),
        inst_n_(chn),
        en_gpio_(en_gpio){;}

    MP1907(hal::AdvancedTimer & timer, const uint8_t pair_index, hal::GpioIntf & en_gpio);

    void init();
    void setDutyRange(const Range2_t<real_t> & _range){duty_range = _range;}

    void enable(bool en = true){
        en_gpio_.write(BoolLevel::from(en));
    }
    MP1907 & operator=(const real_t duty) override;
};

};