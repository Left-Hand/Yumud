#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"

namespace ymd::hal::adc_details{

    void install_pin(const AdcChannelNth channel_nth, const Enable en){
        uint8_t ch_index = (uint8_t)channel_nth;

        if(ch_index > 15) return;

        const auto [gpio_port, pin_nth] = [&] -> std::tuple<GpioPort *, PinNth>{
            if(ch_index <= 7){
                return {&portA, std::bit_cast<PinNth>(uint16_t(1 << ch_index))};
            }else if(ch_index <= 9){
                return {&portB, std::bit_cast<PinNth>(uint16_t(1 << (ch_index - 8)))};
            }else if(ch_index <= 15){
                return {&portC, std::bit_cast<PinNth>(uint16_t(1 << (ch_index - 10)))};
            }else{
                return {nullptr, std::bit_cast<PinNth>(uint16_t(0))};
            }
        }();


        if(gpio_port == nullptr) return;
        Gpio & io = (*gpio_port)[pin_nth];
        if(en == EN)io.inana();
        else io.inflt();
    }

    real_t read_temp(){
        return 25;
    }
}