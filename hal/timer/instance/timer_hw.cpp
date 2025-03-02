#include "timer_hw.hpp"

using namespace ymd::hal;

using IT = TimerIT;


#define ADVANCED_TIMER_IT_TEMPLATE(x)\
void TIM##x##_BRK_IRQHandler(void){\
    timer##x.onBreakInterrupt();\
    TIM_ClearFlag(TIM##x, TIM_IT_Break);\
}\
void TIM##x##_UP_IRQHandler(void){\
    timer##x.onUpdateInterrupt();\
    TIM_ClearFlag(TIM##x, TIM_IT_Update);\
}\
void TIM##x##_TRG_COM_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Trigger)){\
        timer##x.onTriggerInterrupt();\
        TIM_ClearFlag(TIM##x, TIM_IT_Trigger);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_COM)){\
        timer##x.onComInterrupt();\
        TIM_ClearFlag(TIM##x, TIM_IT_COM);\
    }\
}\
void TIM##x##_CC_IRQHandler(void){\
    timer##x.onCCInterrupt();\
}\


#define GENERIC_TIMER_IT_TEMPLATE(x)\
void TIM##x##_IRQHandler(void){\
    timer##x.onItInterrupt();\
}\



#ifdef ENABLE_TIM1
ADVANCED_TIMER_IT_TEMPLATE(1)
#endif

#ifdef ENABLE_TIM2
GENERIC_TIMER_IT_TEMPLATE(2)
#endif

#ifdef ENABLE_TIM3
GENERIC_TIMER_IT_TEMPLATE(3)
#endif

#ifdef ENABLE_TIM4
GENERIC_TIMER_IT_TEMPLATE(4)
#endif

#ifdef ENABLE_TIM5
GENERIC_TIMER_IT_TEMPLATE(5)
#endif

#ifdef ENABLE_TIM8
ADVANCED_TIMER_IT_TEMPLATE(8)
#endif

#ifdef ENABLE_TIM9
ADVANCED_TIMER_IT_TEMPLATE(9)
#endif

#ifdef ENABLE_TIM10
ADVANCED_TIMER_IT_TEMPLATE(10)
#endif