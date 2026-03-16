#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/sdk.hpp"

namespace ymd::hal::adc::details{

void install_pin(const AdcChannelSelection sel){
    auto set_pin_mode_anglog_input = [](const PortSource ps, Nth nth){
        Gpio io = make_gpio(ps, nth);
        io.inana();
    };

    switch(sel){
        case AdcChannelSelection::CH0 ... AdcChannelSelection::CH7:
            // A0 ~ A7
            return set_pin_mode_anglog_input(PortSource::PA, Nth(static_cast<uint8_t>(sel)));
        case AdcChannelSelection::CH8 ... AdcChannelSelection::CH9:
            // B0 ~ B1
            return set_pin_mode_anglog_input(PortSource::PB, Nth((static_cast<uint8_t>(sel) - 8)));
        case AdcChannelSelection::CH10 ... AdcChannelSelection::CH15:
            // C0 ~ C5
            return set_pin_mode_anglog_input(PortSource::PC, Nth((static_cast<uint8_t>(sel) - 10)));

        case AdcChannelSelection::TEMP:
        case AdcChannelSelection::VREF:
            //不是外置引脚，do nothing
            break;
    }
    __builtin_unreachable();

}
}