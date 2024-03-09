#include "gpio.hpp"

// static void setGpioMode(GPIO_TypeDef & instance, const uint8_t & index, const PinMode & mode){

//     uint32_t tempreg = pin_cfg;
//     tempreg &= pin_mask;
//     tempreg |= ((uint8_t)mode << ((pin_index % 8) * 4));
//     pin_cfg = tempreg;

//     if(mode == PinMode::InPullUP){
//         instance -> OUTDR |= pin;
//     }else if(mode == PinMode::InPullDN){
//         instance -> OUTDR &= ~pin;
//     }
// }