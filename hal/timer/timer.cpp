#include "timer.hpp"

using namespace yumud;
void BasicTimer::enableRcc(){
    switch(uint32_t(instance)){
        #ifdef HAVE_TIM1
        case TIM1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
            switch(TIM1_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE);
                    break;
                case 2:
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
                    break;
            }
            break;
        #endif

        #ifdef HAVE_TIM2
        case TIM2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
            switch(TIM2_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
                    break;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
                    break;
            }
            break;
        #endif

        #ifdef HAVE_TIM3
        case TIM3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
            switch(TIM3_REMAP){
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
                    break;
            }
            break;
        #endif

        #ifdef HAVE_TIM4
        case TIM4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
            switch(TIM4_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);
                    break;
            }
            break;
        #endif

        #ifdef HAVE_TIM5
        case TIM5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
            break;
        #endif

        #ifdef HAVE_TIM8
        case TIM8_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
            switch(TIM8_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_TIM8, ENABLE);
                    break;
            }
            break;
        #endif
    }
}

uint BasicTimer::getClk(){
    return TimerUtils::isAdvancedTimer(instance) ? Sys::Clock::getAPB2Freq() : Sys::Clock::getAPB1Freq();
}


void BasicTimer::init(const uint32_t freq, const Mode mode, const bool en){
    this->enableRcc();
    uint32_t raw_period = this->getClk() / freq;

    // TIM_Get_BusFreq(instance);
    // uint32_t raw_period = 144000000 / freq;

    // can`t adaptly select the best frequency by gcd
    // TODO

    uint16_t cycle = 1;
    while(raw_period > 16384 * cycle){
        cycle++;
    }

    if(raw_period / cycle == 0) CREATE_FAULT;

    init(raw_period / cycle, cycle, mode, en);
}

void BasicTimer::init(const uint16_t period, const uint16_t cycle, const Mode mode, const bool en){
    this->enableRcc();

    TIM_InternalClockConfig(instance);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
        .TIM_Prescaler = uint16_t(MAX(cycle - 1, 0)),
        .TIM_CounterMode = (uint16_t)mode,
        .TIM_Period = uint16_t(MAX(period - 1, 0)),
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };

    TIM_TimeBaseInit(instance, &TIM_TimeBaseStructure);
    this->getClk();
    TIM_ClearFlag(instance, 0x1e7f);
    TIM_ClearITPendingBit(instance, 0x00ff);
    enable(en);
}


void BasicTimer::enable(const bool en){
    if(en){
        TIM_Cmd(instance, ENABLE);
        if(TimerUtils::isAdvancedTimer(instance)){
            TIM_CtrlPWMOutputs(instance, ENABLE);
        }
    }else{
        TIM_Cmd(instance, DISABLE);
    }
}

void GenericTimer::initAsEncoder(const Mode mode){
    this->enableRcc();


    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
            .TIM_Prescaler = 0,
            .TIM_CounterMode = (uint16_t)mode,
            .TIM_Period = 65535,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0,
        };

        TIM_TimeBaseInit(instance, &TIM_TimeBaseStructure);
    }


    {
        TIM_ICInitTypeDef TIM_ICInitStruct = {
            .TIM_Channel = TIM_Channel_1,
            .TIM_ICPolarity = TIM_ICPolarity_Rising,
            .TIM_ICSelection = TIM_ICSelection_DirectTI,
            .TIM_ICPrescaler = TIM_ICPSC_DIV1,
            .TIM_ICFilter = 0x0F
        };

        TIM_ICInit(instance,&TIM_ICInitStruct);

        TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
        TIM_ICInit(instance,&TIM_ICInitStruct);
    }

	TIM_EncoderInterfaceConfig(instance,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
    TIM_Cmd(instance, ENABLE);
}

void GenericTimer::enableSingle(const bool _single){
    TIM_SelectOnePulseMode(instance, _single ? TIM_OPMode_Repetitive:TIM_OPMode_Single);
}

void GenericTimer::setTrgoSource(const TrgoSource source){
    TIM_SelectOutputTrigger(instance, (uint8_t)source);
}


void AdvancedTimer::initBdtr(const uint32_t ns, const LockLevel level){

    TIM_BDTRInitTypeDef TIM_BDTRInitStructure{
        .TIM_OSSRState = TIM_OSSRState_Disable,
        .TIM_OSSIState = TIM_OSSIState_Disable,
        .TIM_LOCKLevel = (uint16_t)level,
        .TIM_DeadTime = this->calculateDeadzone(ns),
        .TIM_Break = TIM_Break_Disable,
        .TIM_BreakPolarity = TIM_BreakPolarity_Low,
        .TIM_AutomaticOutput = TIM_AutomaticOutput_Enable
    };

    TIM_BDTRConfig(instance, &TIM_BDTRInitStructure);
}

void AdvancedTimer::setDeadZone(const uint32_t ns){
    uint8_t dead = this->calculateDeadzone(ns);

    uint16_t tempreg = instance->BDTR;
    tempreg &= 0xff00;
    tempreg |= dead;
    instance->BDTR = tempreg;
}

uint8_t AdvancedTimer::calculateDeadzone(const uint ns){
	const uint64_t busFreq = this->getClk();

    uint8_t dead = (ns * (busFreq / 1000000) / 1000);

    if(dead < 128){

    }else if(dead < 256){
        uint8_t head = 0b10000000;
        uint8_t mask = 0b00111111;

        dead = MIN(dead, 254) / 2;
        dead -= 64;
        dead &= mask;
        dead |= head;
    }else if(dead < 509){
        uint8_t head = 0b11000000;
        uint8_t mask = 0b00011111;

        dead = MIN(dead, 504) / 8;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else if(dead < 1009){
        uint8_t head = 0b11100000;
        uint8_t mask = 0b00011111;

        dead = MIN(dead, 1008) / 16;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else{
        dead = 0xff;
    }

    return dead;
}