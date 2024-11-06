#include "timer_hw.hpp"

using namespace yumud;

#define TIMER_IT_BIND_TEMPLATE(x)\
void Timer##x::bindCb(const IT ch, std::function<void(void)> && cb){\
    cbs##x[CTZ((uint8_t)ch)] = cb;\
}\

#define ADVANCED_TIMER_IT_TEMPLATE(x)\
static std::array<std::function<void(void)>, 8> cbs##x = {0};\
Timer##x timer##x;\
TIMER_IT_BIND_TEMPLATE(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Break)){\
        auto & cb = cbs##x[CTZ((uint8_t)TimerUtils::IT::Break)];\
        if(cb) cb();\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Break);\
    }\
}\
__interrupt void TIM##x##_UP_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Update)){\
        auto & cb = cbs##x[CTZ((uint8_t)TimerUtils::IT::Update)];\
        if(cb) cb();\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Update);\
    }\
}\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void){\
    if(TIM_GetITStatus(TIM##x, TIM_IT_Trigger)){\
        auto & cb = cbs##x[CTZ((uint8_t)TimerUtils::IT::Trigger)];\
        if(cb) cb();\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_Trigger);\
    }else if(TIM_GetITStatus(TIM##x, TIM_IT_COM)){\
        auto & cb = cbs##x[CTZ((uint8_t)TimerUtils::IT::COM)];\
        if(cb) cb();\
        TIM_ClearITPendingBit(TIM##x, TIM_IT_COM);\
    }\
}\
__interrupt void TIM##x##_CC_IRQHandler(void){\
    for(uint8_t it = (uint8_t)TimerUtils::IT::CC1; it <= (uint8_t)TimerUtils::IT::CC4; it <<= 1){\
        if(TIM_GetITStatus(TIM##x, it)){\
            auto & cb = cbs##x[CTZ(it)];\
            if(cb) cb();\
            TIM_ClearITPendingBit(TIM##x, it);\
        }\
    }\
}\
}\
\

#define GENERIC_TIMER_IT_TEMPLATE(x)\
static std::array<std::function<void(void)>, 8> cbs##x = {0};\
Timer##x timer##x;\
TIMER_IT_BIND_TEMPLATE(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void){\
    for(uint8_t it = (uint8_t)TimerUtils::IT::Update; it <= (uint8_t)TimerUtils::IT::CC4; it <<= 1){\
        if(TIM_GetITStatus(TIM##x, it)){\
            auto & cb = cbs##x[CTZ(it)];\
            if(cb) cb();\
            TIM_ClearITPendingBit(TIM##x, it);\
        }\
    }\
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