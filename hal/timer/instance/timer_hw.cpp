#include "timer_hw.hpp"
#include "core/sdk.hpp"

using namespace ymd::hal;

using IT = TimerIT;


#define ADVANCED_TIMER_IT_TEMPLATE(x)\
void TIM##x##_BRK_IRQHandler(void){\
    timer##x.accept_interrupt(IT::Break);\
    TIM_ClearFlag(TIM##x, TIM_IT_Break);\
}\
void TIM##x##_UP_IRQHandler(void){\
    timer##x.accept_interrupt(IT::Update);\
    TIM_ClearFlag(TIM##x, TIM_IT_Update);\
}\
void TIM##x##_TRG_COM_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Trigger)){\
        timer##x.accept_interrupt(IT::Trigger);\
        TIM_ClearFlag(TIM##x, TIM_IT_Trigger);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_COM)){\
        timer##x.accept_interrupt(IT::COM);\
        TIM_ClearFlag(TIM##x, TIM_IT_COM);\
    }\
}\
void TIM##x##_CC_IRQHandler(void){\
    timer##x.on_cc_interrupt();\
}\


#define GENERIC_TIMER_IT_TEMPLATE(x)\
void TIM##x##_IRQHandler(void){\
    timer##x.on_interrupt();\
}\



#ifdef TIM1_PRESENT
ADVANCED_TIMER_IT_TEMPLATE(1)
#endif

#ifdef TIM2_PRESENT
GENERIC_TIMER_IT_TEMPLATE(2)
#endif

#ifdef TIM3_PRESENT
GENERIC_TIMER_IT_TEMPLATE(3)
#endif

#ifdef TIM4_PRESENT
GENERIC_TIMER_IT_TEMPLATE(4)
#endif

#ifdef TIM5_PRESENT
GENERIC_TIMER_IT_TEMPLATE(5)
#endif

#ifdef TIM8_PRESENT
ADVANCED_TIMER_IT_TEMPLATE(8)
#endif

#ifdef TIM9_PRESENT
ADVANCED_TIMER_IT_TEMPLATE(9)
#endif

#ifdef TIM10_PRESENT
ADVANCED_TIMER_IT_TEMPLATE(10)
#endif