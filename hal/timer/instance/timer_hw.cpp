#include "timer_hw.hpp"

using namespace ymd;

using IT = TimerUtils::IT;


#define ADVANCED_TIMER_IT_TEMPLATE(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void){\
    timer##x.onBreakHandler();\
}\
\
__interrupt void TIM##x##_UP_IRQHandler(void){\
    timer##x.onUpdateHandler();\
}\
\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void){\
    timer##x.onTriggerComHandler();\
}\
\
__interrupt void TIM##x##_CC_IRQHandler(void){\
    timer##x.onCCHandler();\
}\
}\
\

#define GENERIC_TIMER_IT_TEMPLATE(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void){\
    timer##x.onItHandler();\
}\
}\
\


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