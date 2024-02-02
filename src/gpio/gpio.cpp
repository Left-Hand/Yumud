// #include "gpio.hpp"

// void Port::classify(){
//     if(((uint32_t)base > GPIOA_BASE) && ((uint32_t)base < GPIOC_BASE)){
//         index = ((uint32_t)base - GPIOA_BASE) / (GPIOB_BASE) + 1;
//         periph_c = RCC_APB2Periph_GPIOA << (index - 1);
//     }else{
//         while(1);
//     }
// }

// void Port::init(){
//     classify();
//     RCC_APB2PeriphClockCmd(periph_c, ENABLE);
// }

// void Pin::config(GPIOMode_TypeDef _mode, GPIOSpeed_TypeDef _speed){
//     mode = _mode;
//     speed = _speed;
    
//     init();
// }

// void Pin::init(){

//     RCC_APB2PeriphClockCmd(classifyPort(base), ENABLE);

//     GPIO_InitTypeDef  GPIO_InitStructure = {
//         .GPIO_Pin = pin,
//         .GPIO_Speed = speed,
//         .GPIO_Mode = mode
//     };
//     GPIO_Init(base, &GPIO_InitStructure);
// }

// uint16_t classifyPort(GPIO_TypeDef* _base){
//     return (RCC_APB2Periph_GPIOA << ((uint32_t)_base - GPIOA_BASE) / (GPIOB_BASE));}
// void enablePort(GPIO_TypeDef* _base){
//     RCC_APB2PeriphClockCmd(classifyPort(_base), ENABLE);}
// void pinMode(Pin & _pin, GPIOMode_TypeDef _mode){_pin.config(_mode);}
// void digitalWrite(Pin & _pin, bool _state){_pin = _state;}
// bool digitalRead(Pin & _pin){return _pin.read();}
