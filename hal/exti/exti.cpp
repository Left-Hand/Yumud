#include "exti.hpp"
#include "gpio/gpio.hpp"

using namespace ymd::hal;

using TrigEdge = ExtiTrigEdge;
using TrigMode = ExtiTrigMode;
using TrigSource = ExtiTrigSource;

ExtiChannel::ExtiChannel(
    const TrigSource source, 
    const NvicPriority priority,
    const TrigEdge edge, 
    const TrigMode mode
):

    source_(source), 
    p_gpio_(nullptr), 
    gpio_mode_(GpioMode::InAnalog),
    priority_(priority), 
    edge_(edge), 
    mode_(mode){;}


ExtiChannel::ExtiChannel(
    Gpio & gpio, 
    const NvicPriority priority,
    const TrigEdge edge,
    const TrigMode mode
):
    source_(map_pinsource_to_trigsource(gpio.pin())), 
    p_gpio_(&gpio),
    gpio_mode_(map_edge_to_gpiomode(edge)),
    priority_(priority), 
    edge_(edge),
    mode_(mode){;}


void ExtiChannel::init(){
    if(p_gpio_){
        p_gpio_->set_mode(gpio_mode_);
        if(p_gpio_->is_valid()){
            GPIO_EXTILineConfig(
                std::bit_cast<uint8_t>(p_gpio_->port()), 
                std::bit_cast<uint8_t>(p_gpio_->index())
            );
        }
    }

    EXTI_InitTypeDef EXTI_InitStructure{
        .EXTI_Line = (uint32_t)source_,
        .EXTI_Mode = (EXTIMode_TypeDef)mode_,
        .EXTI_Trigger = (EXTITrigger_TypeDef)edge_,
        .EXTI_LineCmd = ENABLE
    };

    EXTI_Init(&EXTI_InitStructure);

    if(mode_ == TrigMode::Interrupt){
        enable_it(EN);
    }
}



static std::array<std::function<void(void)>, 21> funcs;

#define EXTI_INTERRUPT_CONTENT_TEMPLATE(n)\
if(EXTI_GetITStatus((uint32_t)ExtiChannel::TrigSource::_##n)){\
    if(funcs[n]) funcs[n]();\
    EXTI_ClearITPendingBit((uint32_t)ExtiChannel::TrigSource::_##n);\
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