#pragma once

#include "hal/timer/timer.hpp"


#define ADVANCED_TIMER_TEMPLATE(x)\
class Timer##x:public yumud::AdvancedTimer{\
public:\
    Timer##x():AdvancedTimer(TIM##x){;}\
    void bindCb(const IT ch, std::function<void(void)> && cb) override;\
};\
\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void);\
__interrupt void TIM##x##_UP_IRQHandler(void);\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void);\
__interrupt void TIM##x##_CC_IRQHandler(void);\
}\
\
extern Timer##x timer##x;\

#define GENERIC_TIMER_TEMPLATE(x)\
class Timer##x:public yumud::GenericTimer{\
public:\
    Timer##x():GenericTimer(TIM##x){;}\
    void bindCb(const IT ch, std::function<void(void)> && cb) override;\
};\
\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\
\
extern Timer##x timer##x;\

#define BASIC_TIMER_TEMPLATE(x)\
class Timer##x:public yumud::BasicTimer{\
public:\
    Timer##x():BasicTimer(TIM##x){;}\
    void bindCb(const IT ch, std::function<void(void)> && cb) override;\
};\
\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\
\
extern Timer##x timer##x;\


#ifdef ENABLE_TIM1
ADVANCED_TIMER_TEMPLATE(1)
#endif

#ifdef ENABLE_TIM2
GENERIC_TIMER_TEMPLATE(2)
#endif

#ifdef ENABLE_TIM3
GENERIC_TIMER_TEMPLATE(3)
#endif

#ifdef ENABLE_TIM4
GENERIC_TIMER_TEMPLATE(4)
#endif

#ifdef ENABLE_TIM5
GENERIC_TIMER_TEMPLATE(5)
#endif

#ifdef ENABLE_TIM6
BASIC_TIMER_TEMPLATE(6)
#endif

#ifdef ENABLE_TIM7
BASIC_TIMER_TEMPLATE(7)
#endif

#ifdef ENABLE_TIM8
ADVANCED_TIMER_TEMPLATE(8)
#endif

#ifdef ENABLE_TIM9
ADVANCED_TIMER_TEMPLATE(9)
#endif

#ifdef ENABLE_TIM10
ADVANCED_TIMER_TEMPLATE(10)
#endif