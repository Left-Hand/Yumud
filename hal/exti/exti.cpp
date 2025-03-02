#include "exti.hpp"

using namespace ymd::hal;

ExtiChannel::ExtiChannel(const Line _line, const NvicPriority & _priority,
        const Trigger _trigger, const Mode _mode):

    line(_line), 
    gpio(nullptr), 
    gpio_mode(GpioMode::InAnalog),
    priority(_priority), 
    trigger(_trigger), 
    mode(_mode){;}


ExtiChannel::ExtiChannel(Gpio & _gpio, const NvicPriority & _priority,
        const Trigger _trigger,  const Mode _mode):

    line(from_gpio_to_line(_gpio)), 
    gpio(&_gpio),
    gpio_mode(
        (trigger == Trigger::Dual)? GpioMode::InFloating : 
        ((trigger == Trigger::Rising)? GpioMode::InPullDN : GpioMode::InPullUP)
    ),
    priority(_priority), 
    trigger(_trigger),
    mode(_mode){;}


ExtiChannel::Source ExtiChannel::from_gpio_to_source(const Gpio & gpio){
    switch((uint32_t)gpio.instance){
        default:
        #ifdef ENABLE_GPIOA
        case GPIOA_BASE:
            return Source::PA;
        #endif
        #ifdef ENABLE_GPIOB
        case GPIOB_BASE:
            return Source::PB;
        #endif
        #ifdef ENABLE_GPIOC
        case GPIOC_BASE:
            return Source::PC;
        #endif
        #ifdef ENABLE_GPIOD
        case GPIOD_BASE:
            return Source::PD;
        #endif
        #ifdef ENABLE_GPIOE
        case GPIOE_BASE:
            return Source::PE;
        #endif
        #ifdef ENABLE_GPIOF
        case GPIOF_BASE:
            return Source::PF;
        #endif
    }
}


ExtiChannel::Line ExtiChannel::from_gpio_to_line(const Gpio & gpio){
    return gpio.isValid() ? (Line)(1 << gpio.getIndex()) : (Line::_None);
}


IRQn ExtiChannel::from_line_to_irqn(const Line line){
    switch(line){
        case Line::_0: return EXTI0_IRQn;
        case Line::_1: return EXTI1_IRQn;
        case Line::_2: return EXTI2_IRQn;
        case Line::_3: return EXTI3_IRQn;
        case Line::_4: return EXTI4_IRQn;
        case Line::_5 ... Line::_9: return EXTI9_5_IRQn;
        case Line::_10 ... Line::_15: return EXTI15_10_IRQn;
        default: return IRQn(0);
    }
}


void ExtiChannel::init(){
    if(gpio){
        gpio->setMode(gpio_mode);
        if(gpio->getIndex() > 0) GPIO_EXTILineConfig((uint8_t)from_gpio_to_source(*gpio), gpio->getIndex());
    }

    EXTI_InitTypeDef EXTI_InitStructure = {0};

    EXTI_InitStructure.EXTI_Line = (uint32_t)line;
    EXTI_InitStructure.EXTI_Mode = (EXTIMode_TypeDef)mode;
    EXTI_InitStructure.EXTI_Trigger = (EXTITrigger_TypeDef)trigger;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    if(mode == Mode::Interrupt){
        enableIt(true);
    }
}



static std::array<std::function<void(void)>, 21> funcs;

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