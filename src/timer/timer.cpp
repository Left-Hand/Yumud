#include "timer.hpp"

static void TIM_RCC_ON(TIM_TypeDef * TimBase){
    if (TimBase == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (TimBase == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    } else if (TimBase == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (TimBase == TIM4){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    }
}

void BasicTimer::init(const uint16_t arr, const uint16_t psc, const TimerMode mode){
    TIM_RCC_ON(base);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = (uint16_t)mode;
    TIM_TimeBaseInit(base,&TIM_TimeBaseStructure);
}
void BasicTimer::enable(const bool en){
    if(en){
        TIM_Cmd(base, ENABLE);
        if(base == TIM1){
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
        }
    }else{
        TIM_Cmd(base, DISABLE);
    }
}
