#include "exti.hpp"

static std::array<std::function<void(void)>, 21> funcs;
void ExtiChannel::bindCb(const std::function<void(void)> & func){
    funcs[CTZ((uint32_t)line)] = func;
}

#define EXTI_INTERRUPT_CONTENT_TEMPLATE(n)\
if(EXTI_GetITStatus((uint32_t)ExtiChannel::Line::_##n)){\
    if(funcs[n]) funcs[n]();\
    EXTI_ClearITPendingBit((uint32_t)ExtiChannel::Line::_##n);\
}\

#define EXTI_INTERRUPT_HANDLER_TEMPLATE_BEGIN(n)\
__interrupt void EXTI##n##_IRQHandler(void){\

#define EXTI_INTERRUPT_HANDLER_TEMPLATE_END }

#define EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(n)\
    EXTI_INTERRUPT_HANDLER_TEMPLATE_BEGIN(n)\
    EXTI_INTERRUPT_CONTENT_TEMPLATE(n)\
    EXTI_INTERRUPT_HANDLER_TEMPLATE_END\

EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(0)
EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(1)
EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(2)
EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(3)
EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE(4)

EXTI_INTERRUPT_HANDLER_TEMPLATE_BEGIN(9_5)
EXTI_INTERRUPT_CONTENT_TEMPLATE(5)
EXTI_INTERRUPT_CONTENT_TEMPLATE(6)
EXTI_INTERRUPT_CONTENT_TEMPLATE(7)
EXTI_INTERRUPT_CONTENT_TEMPLATE(8)
EXTI_INTERRUPT_CONTENT_TEMPLATE(9)
EXTI_INTERRUPT_HANDLER_TEMPLATE_END

EXTI_INTERRUPT_HANDLER_TEMPLATE_BEGIN(15_10)

EXTI_INTERRUPT_CONTENT_TEMPLATE(10)
EXTI_INTERRUPT_CONTENT_TEMPLATE(11)
EXTI_INTERRUPT_CONTENT_TEMPLATE(12)
EXTI_INTERRUPT_CONTENT_TEMPLATE(13)
EXTI_INTERRUPT_CONTENT_TEMPLATE(14)
EXTI_INTERRUPT_CONTENT_TEMPLATE(15)
EXTI_INTERRUPT_HANDLER_TEMPLATE_END

#undef EXTI_INTERRUPT_HANDLER_TEMPLATE_BEGIN
#undef EXTI_INTERRUPT_HANDLER_TEMPLATE_SINGLE
#undef EXTI_INTERRUPT_HANDLER_TEMPLATE_END