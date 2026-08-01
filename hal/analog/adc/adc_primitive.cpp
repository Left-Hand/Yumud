#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "core/sdk.hpp"

namespace ymd::hal::adc::details{


void install_pin(const AdcChannelSelection sel){

    PortSource port = PortSource::PA;
    size_t nth = 0;
    
    {
        switch(sel){
            case AdcChannelSelection::CH0 ... AdcChannelSelection::CH7:
                // A0 ~ A7
                port = PortSource::PA;
                nth = size_t(static_cast<uint8_t>(sel));
                break;
            case AdcChannelSelection::CH8 ... AdcChannelSelection::CH9:
                // B0 ~ B1
                port = PortSource::PB;
                nth = size_t((static_cast<uint8_t>(sel) - 8));
                break;
            case AdcChannelSelection::CH10 ... AdcChannelSelection::CH15:
                // C0 ~ C5
                port = PortSource::PC;
                nth = size_t((static_cast<uint8_t>(sel) - 10));
                break;

            case AdcChannelSelection::TEMP:
            case AdcChannelSelection::VREF:
                //不是外置引脚，do nothing
                return;
        }
    };

    Gpio io = make_gpio(port, Nth(nth));
    io.inana();

}
}