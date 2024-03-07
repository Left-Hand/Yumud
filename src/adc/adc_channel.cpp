#include "adc_channel.hpp"

void AdcChannelOnChip::installToPin(const bool en){

    uint8_t ch_index = (uint8_t)channel;

    if(ch_index > 15) return;

    GPIO_TypeDef * gpio_port;
    uint16_t gpio_pin = 0;

    if(ch_index <= 7){
        gpio_port = GPIOA;
        gpio_pin = 1 << ch_index;
    }else if(ch_index <= 9){
        gpio_port = GPIOB;
        gpio_pin = 1 << (ch_index - 8);
    }else if(ch_index <= 15){
        gpio_port = GPIOC;
        gpio_pin = 1 << (ch_index - 10);
    }

    Gpio io = Gpio(gpio_port, gpio_pin);
    if(en)io.InAnalog();
    else io.InFloating();

}