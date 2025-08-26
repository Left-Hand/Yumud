#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"

namespace ymd::hal::adc_details{

    void install_pin(const AdcChannelNth channel, const Enable en){
        uint8_t ch_index = (uint8_t)channel;

        if(ch_index > 15) return;

        GpioPort * gpio_port = nullptr;
        PinNth pin = PinNth::None;

        if(ch_index <= 7){
            gpio_port = &portA;
            pin = std::bit_cast<PinNth>(uint16_t(1 << ch_index));
        }else if(ch_index <= 9){
            gpio_port = &portB;
            pin = std::bit_cast<PinNth>(uint16_t(1 << (ch_index - 8)));
        }else if(ch_index <= 15){
            gpio_port = &portC;
            pin = std::bit_cast<PinNth>(uint16_t(1 << (ch_index - 10)));
        }

        Gpio & io = (*gpio_port)[pin];
        if(en == EN)io.inana();
        else io.inflt();
    }

    real_t read_temp(){
        return 25;
    }
}