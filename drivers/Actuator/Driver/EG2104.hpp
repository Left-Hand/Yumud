#pragma once

#include "CoilDriver.hpp"
#include "../../hal/timer/timer_oc.hpp"


class EG2103:public Coil2Driver{
protected:
    TimerOC & inst_;
    Range duty_range;
public:
    EG2103(TimerOC & ch);
    EG2103(TimerOC & ch, TimerOCN & chn);

    void setDutyRange(const Range & _range);

    EG2103 & operator=(const real_t duty){

    }
};

class EG2104:public Coil2Driver{

};