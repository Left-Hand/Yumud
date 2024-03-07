#ifndef __TIM1_HPP__

#define __TIM1_HPP__

#include "../timer.hpp"

class Timer1:public AdvancedTimer{
public:
    Timer1():AdvancedTimer(TIM1){;}
};

#ifdef HAVE_TIM1

extern "C"{
__interrupt void TIM1_BRK_IRQHandler(void);
__interrupt void TIM1_UP_IRQHandler(void);
__interrupt void TIM1_TRG_COM_IRQHandler(void);
__interrupt void TIM1_CC_IRQHandler(void);
}

extern Timer1 timer1;
#endif

#endif