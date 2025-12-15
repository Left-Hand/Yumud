#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/sdk.hpp"

namespace ymd::hal::adc::details{

void install_pin(const AdcChannelSelection sel){
    auto make_pin_to_anglog_input = [](GPIO_TypeDef * gpio_port, Nth nth){
        Gpio io = make_gpio(gpio_port, nth);
        io.inana();
    };

    switch(sel){
        case AdcChannelSelection::CH0 ... AdcChannelSelection::CH7:
            return make_pin_to_anglog_input(GPIOA, Nth(static_cast<uint8_t>(sel)));
        case AdcChannelSelection::CH8 ... AdcChannelSelection::CH9:
            return make_pin_to_anglog_input(GPIOB, Nth((static_cast<uint8_t>(sel) - 8)));
        case AdcChannelSelection::CH10 ... AdcChannelSelection::CH15:
            return make_pin_to_anglog_input(GPIOC, Nth((static_cast<uint8_t>(sel) - 10)));

        case AdcChannelSelection::TEMP:
        case AdcChannelSelection::VREF:
            //不是外置引脚，do nothing
            break;
    }
    __builtin_unreachable();

}
}