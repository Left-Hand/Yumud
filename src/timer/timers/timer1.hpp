#ifndef __TIM1_HPP__

#define __TIM1_HPP__

#include "../timer.hpp"

class Timer1:public AdvancedTimer{
public:
    Timer1():AdvancedTimer(TIM1){;}
};

#ifdef HAVE_TIM1
extern Timer1 timer1;
#endif

#endif