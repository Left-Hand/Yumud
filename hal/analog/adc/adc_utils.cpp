#include "adc_utils.hpp"
#include "hal/gpio/gpio_port.hpp"

namespace ymd::hal::adc_details{

    void install_pin(const AdcChannelSelection channel_nth, const Enable en){
        uint8_t ch_index = std::bit_cast<uint8_t>(channel_nth);

        if(ch_index > 15) return;

        const auto [gpio_port, nth] = [&] -> std::tuple<GPIO_TypeDef *, Nth>{
            if(ch_index <= 7){
                return {GPIOA, Nth(ch_index)};
            }else if(ch_index <= 9){
                return {GPIOB, Nth((ch_index - 8))};
            }else if(ch_index <= 15){
                return {GPIOC, Nth((ch_index - 10))};
            }else{
                return {nullptr, Nth(0)};
            }
        }();


        if(gpio_port == nullptr) return;
        Gpio io = make_gpio(gpio_port, nth);
        if(en == EN)io.inana();
        else io.inflt();
    }

    real_t read_temp(){
        return 25;
    }
}