#pragma once

#include "../CoilDriver.hpp"
#include "../../types/range/range_t.hpp"

#include <optional>

struct TimerOC;
struct TimerOCN;

class EG2103:public Coil2Driver{
protected:
    TimerOC & inst_;
    TimerOCN * p_inst_n_;
    Range duty_range = {real_t(0.03), real_t(0.97)};
public:
    EG2103(TimerOC & ch):inst_(ch), p_inst_n_(nullptr){;}
    EG2103(TimerOC & ch, TimerOCN & chn):inst_(ch), p_inst_n_(&chn){;}

    void init();

    void setDutyRange(const Range & _range){duty_range = _range;}

    EG2103 & operator=(const real_t duty) override;
};
