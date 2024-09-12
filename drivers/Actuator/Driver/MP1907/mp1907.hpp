#pragma once

#include "../CoilDriver.hpp"
#include "types/range/range_t.hpp"

#include <optional>

struct TimerOC;
struct TimerOCN;
struct AdvancedTimer;

class MP1907:public Coil2Driver{
protected:
    TimerOC & inst_;
    TimerOCN & inst_n_;
    GpioConcept * en_gpio_ = nullptr;
    Range duty_range = {real_t(0.03), real_t(0.97)};

public:
    MP1907(TimerOC & ch, TimerOCN & chn, GpioConcept & en_gpio):
        inst_(ch),
        inst_n_(chn),
        en_gpio_(&en_gpio){;}

    MP1907(AdvancedTimer & timer, const uint8_t pair_index, GpioConcept & en_gpio);

    void init();
    void setDutyRange(const Range & _range){duty_range = _range;}

    void enable(bool en = true){
        if(en_gpio_){
            en_gpio_->write(en);
        }
    }
    MP1907 & operator=(const real_t duty) override;
};
