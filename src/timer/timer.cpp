#include "timer.hpp"

static void TIM_RCC_ON(TIM_TypeDef * base){
    if (base == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (base == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    } else if (base == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (base == TIM4){
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

void GenericTimer::initAsEncoder(const TimerMode mode){
    TIM_RCC_ON(base);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = (uint16_t)mode;
    TIM_TimeBaseInit(base,&TIM_TimeBaseStructure);

	TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0xF;


	TIM_ICInit(base,&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;

	TIM_ICInit(base,&TIM_ICInitStruct);

	TIM_EncoderInterfaceConfig(base,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);

    TIM_Cmd(base, ENABLE);
}

void GenericTimer::enableSingle(const bool _single){
    TIM_SelectOnePulseMode(base, _single ? TIM_OPMode_Repetitive:TIM_OPMode_Single);
}

uint8_t AdvancedTimer::caculate_dead_zone(uint32_t ns){
	RCC_ClocksTypeDef RCC_CLK;
    RCC_GetClocksFreq(&RCC_CLK);
	uint64_t busFreq = RCC_CLK.PCLK2_Frequency;

    uint8_t dead = (ns * busFreq / 1E9);
    uint8_t head = 0;
    uint8_t mask = 0xff;

    if(dead < 128){

    }else if(dead < 256){
        head = 0b10000000;
        mask = 0b00111111;

        dead = MIN(dead, 254) / 2;
        dead -= 64;
        dead &= mask;
        dead |= head;
    }else if(dead < 509){
        head = 0b11000000;
        mask = 0b00011111;

        dead = MIN(dead, 504) / 8;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else if(dead < 1009){
        head = 0b11100000;
        mask = 0b00011111;

        dead = MIN(dead, 1008) / 16;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else{
        dead = 0xff;
    }
    return dead;
}

void AdvancedTimer::initBdtr(const LockLevel level, const uint32_t ns){

    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
    TIM_BDTRInitStructure.TIM_LOCKLevel = (uint16_t)level;
    TIM_BDTRInitStructure.TIM_DeadTime = caculate_dead_zone(ns);
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(base, &TIM_BDTRInitStructure);
}

void AdvancedTimer::setDeadZone(const uint32_t ns){
    uint8_t dead = caculate_dead_zone(ns);

    uint16_t tempreg = base->BDTR;
    tempreg &= 0xff00;
    tempreg |= dead;
    base->BDTR = tempreg;
}