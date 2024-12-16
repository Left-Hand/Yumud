#include "ad7607.hpp"


using namespace ymd::drivers;
using namespace ymd;



void AD7607::reset(void){
    /*! ___|-----|________  >= 50ns */
    rst_gpio.clr();
    rst_gpio.set();
    delay(1);
    rst_gpio.clr();
}
 
void AD7607::init(void){
    rst_gpio.outpp(0);
    reset();
}
 
void AD7607::start(void){
    // HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);//启动AD7607转换
}
 
void AD7607::stop(void){
    // HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_1);//停止AD7607转换
}
 
void AD7607::BusyIrqCallback(const uint16_t *ad7606Val,uint8_t ad7606Chl){
    // spi_drv.re
}
 
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//PA5 中断
// {
//     if(GPIO_Pin == AD7607Irq_Pin){
//         //read AD7607
//         if(nums < SAMPLING_POINTS){
//             AD7607BusyIrqCallback(ad7606Buff[nums],SAMPLING_CHANNEL);
//             nums++;
//             ad7606SamplingDoneFlag = 0;
//         }
//         else{
//             ad7606SamplingDoneFlag = 1;
//         }
//     }
// }

real_t AD7607::conv(uint16_t bin){
    int _val;
    real_t adcValue;
    _val = bin&0x8000 ? (-((~bin+1)&0x7fff)) : bin;
    adcValue = _val * 5 /32768;
    return adcValue;
}