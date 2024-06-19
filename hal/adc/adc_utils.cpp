#include "adc_utils.hpp"

namespace AdcUtils{

    void installPin(const Channel channel, const bool en){
        uint8_t ch_index = (uint8_t)channel;

        if(ch_index > 15) return;

        Port * gpio_port = nullptr;
        Pin gpio_pin = Pin::None;

        if(ch_index <= 7){
            gpio_port = &portA;
            gpio_pin = (Pin)(1 << ch_index);
        }else if(ch_index <= 9){
            gpio_port = &portB;
            gpio_pin = (Pin)(1 << (ch_index - 8));
        }else if(ch_index <= 15){
            gpio_port = &portC;
            gpio_pin = (Pin)(1 << (ch_index - 10));
        }

        Gpio & io = (*gpio_port)[gpio_pin];
        if(en)io.InAnalog();
        else io.InFloating();
    }

    real_t getTemperature(){
        return 25;
    }
}