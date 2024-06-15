#include "timer.hpp"

static void TIM_RCC_ON(TIM_TypeDef * instance){
    if (instance == TIM1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    } else if (instance == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        GPIO_PinRemapConfig(TIM2_REMAP, TIM2_REMAP_ENABLE);
    } else if (instance == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (instance == TIM4){
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    }
}

static uint32_t TIM_Get_BusFreq(TIM_TypeDef * instance){
    bool isAbp2 = false;
    switch(uint32_t(instance)){

        case TIM1_BASE:
            isAbp2 = true;
            break;
        default:
        case TIM2_BASE:
            isAbp2 = false;
            break;
        case TIM3_BASE:
            isAbp2 = false;
            break;
        case TIM4_BASE:
            isAbp2 = false;
            break;
    }


    RCC_ClocksTypeDef clock;
    RCC_GetClocksFreq(&clock);

    if (isAbp2) {
        return clock.PCLK2_Frequency;
        // return SystemCoreClock;
    } else {
        return clock.PCLK1_Frequency;
        // return SystemCoreClock;
    }
}


void BasicTimer::init(const uint32_t & freq, const Mode & mode, const bool & en){
    TIM_RCC_ON(instance);
    uint32_t raw_period = TIM_Get_BusFreq(instance) / freq;
    // TIM_Get_BusFreq(instance);
    // uint32_t raw_period = 144000000 / freq;

    // can`t adaptly select the best frequency by gcd
    // TODO

    uint16_t cycle = 1;
    while(raw_period / cycle > 16384){
        cycle++;
    }

    init(raw_period / cycle, cycle, mode, en);
}

void BasicTimer::init(const uint16_t & period, const uint16_t & cycle, const Mode & mode, const bool & en){
    TIM_RCC_ON(instance);
    TIM_InternalClockConfig(instance);
    TIM_DeInit(instance);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = period - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = cycle - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = (uint16_t)mode;
    TIM_TimeBaseInit(instance,&TIM_TimeBaseStructure);
    TIM_Get_BusFreq(instance);
    TIM_ClearFlag(instance, 0x1e7f);
    TIM_ClearITPendingBit(instance, 0x00ff);
    enable(en);
}



void BasicTimer::enable(const bool & en){
    if(en){
        TIM_Cmd(instance, ENABLE);
        if(instance == TIM1){
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
        }
    }else{
        TIM_Cmd(instance, DISABLE);
    }
}

void GenericTimer::initAsEncoder(const Mode mode){
    TIM_RCC_ON(instance);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = (uint16_t)mode;
    TIM_TimeBaseInit(instance,&TIM_TimeBaseStructure);

	TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0xF;


	TIM_ICInit(instance,&TIM_ICInitStruct);
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(instance,&TIM_ICInitStruct);
	TIM_EncoderInterfaceConfig(instance,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
    TIM_Cmd(instance, ENABLE);
}

void GenericTimer::enableSingle(const bool _single){
    TIM_SelectOnePulseMode(instance, _single ? TIM_OPMode_Repetitive:TIM_OPMode_Single);
}

uint8_t AdvancedTimer::caculate_dead_zone(uint32_t ns){
	RCC_ClocksTypeDef RCC_CLK;
    RCC_GetClocksFreq(&RCC_CLK);
	uint64_t busFreq = RCC_CLK.PCLK2_Frequency;

    uint8_t dead = (ns * (busFreq / 1000000) / 1000);
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

void AdvancedTimer::initBdtr(const uint32_t & ns, const LockLevel & level){

    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
    TIM_BDTRInitStructure.TIM_LOCKLevel = (uint16_t)level;
    TIM_BDTRInitStructure.TIM_DeadTime = caculate_dead_zone(ns);
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(instance, &TIM_BDTRInitStructure);
}

void AdvancedTimer::setDeadZone(const uint32_t & ns){
    uint8_t dead = caculate_dead_zone(ns);

    uint16_t tempreg = instance->BDTR;
    tempreg &= 0xff00;
    tempreg |= dead;
    instance->BDTR = tempreg;
}