#ifndef __TIMHW_HPP__

#define __TIMHW_HPP__

#include "../timer.hpp"




#define ADVANCED_TIMER_TEMPLATE(x)\
class Timer##x:public AdvancedTimer{\
public:\
    Timer##x():AdvancedTimer(TIM##x){;}\
};\
\
extern "C"{\
__interrupt  __fast_inline void TIM##x##_BRK_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Break)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Break);\
    }\
}\
__interrupt  __fast_inline void TIM##x##_UP_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Update)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Update);\
    }\
}\
__interrupt  __fast_inline void TIM##x##_TRG_COM_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Trigger)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Trigger);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_COM)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_COM);\
    }\
}\
__interrupt  __fast_inline void TIM##x##_CC_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_CC1)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC1);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC2)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC2);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC3)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC3);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC4)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC4);\
    }\
}\
}\
\
extern Timer##x timer##x;\

#define GENERIC_TIMER_TEMPLATE(x)\
class Timer##x:public GenericTimer{\
public:\
    Timer##x():GenericTimer(TIM##x){;}\
};\
\
extern "C"{\
__interrupt  __fast_inline void TIM##x##_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_CC1)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC1);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC2)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC2);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC3)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC3);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_CC4)){\
\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_CC4);\
    }\
}\
}\
\
extern Timer##x timer##x;\

#ifdef HAVE_TIM1
ADVANCED_TIMER_TEMPLATE(1)
#endif

#ifdef HAVE_TIM2
GENERIC_TIMER_TEMPLATE(2)
#endif

#endif