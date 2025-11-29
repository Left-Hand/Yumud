#include "timer_hw.hpp"

using namespace ymd::hal;

using IT = TimerIT;


#define ADVANCED_TIMER_IT_TEMPLATE(x)\
void TIM##x##_BRK_IRQHandler(void){\
    timer##x.invoke_callback<IT::Break>();\
    TIM_ClearFlag(TIM##x, TIM_IT_Break);\
}\
void TIM##x##_UP_IRQHandler(void){\
    timer##x.invoke_callback<IT::Update>();\
    TIM_ClearFlag(TIM##x, TIM_IT_Update);\
}\
void TIM##x##_TRG_COM_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Trigger)){\
        timer##x.invoke_callback<IT::Trigger>();\
        TIM_ClearFlag(TIM##x, TIM_IT_Trigger);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_COM)){\
        timer##x.invoke_callback<IT::COM>();\
        TIM_ClearFlag(TIM##x, TIM_IT_COM);\
    }\
}\
void TIM##x##_CC_IRQHandler(void){\
    timer##x.on_cc_interrupt();\
}\


#define GENERIC_TIMER_IT_TEMPLATE(x)\
void TIM##x##_IRQHandler(void){\
    timer##x.on_it_interrupt();\
}\



#ifdef ENABLE_TIM1
void TIM1_BRK_IRQHandler(void){
    timer1.invoke_callback<IT::Break>();
    TIM_ClearFlag(TIM1, TIM_IT_Break);
}
void TIM1_UP_IRQHandler(void){
    timer1.invoke_callback<IT::Update>();
    TIM_ClearFlag(TIM1, TIM_IT_Update);
}
void TIM1_TRG_COM_IRQHandler(void){
    if(TIM_GetITStatus(TIM1, TIM_IT_Trigger)){
        timer1.invoke_callback<IT::Trigger>();
        TIM_ClearFlag(TIM1, TIM_IT_Trigger);
    }else if(TIM_GetITStatus(TIM1, TIM_IT_COM)){
        timer1.invoke_callback<IT::COM>();
        TIM_ClearFlag(TIM1, TIM_IT_COM);
    }
}
void TIM1_CC_IRQHandler(void){
    timer1.on_cc_interrupt();
}

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