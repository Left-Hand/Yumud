#include "timer.hpp"
#include "sys/core/system.hpp"

using namespace ymd;
using namespace ymd::TimerUtils::internal;

// #define TIM_DEBUG

#ifdef TIM_DEBUG
__inline void TIM_ASSERT(bool x){
    if(!x) HALT;
}

#else
#define TIM_ASSERT(x)
#endif

void BasicTimer::enableRcc(const bool en){
    switch(uint32_t(instance)){
        #ifdef ENABLE_TIM1
        case TIM1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, en);
            switch(TIM1_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, en);
                    break;
                case 2:
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, en);
                    break;
            }
            break;
        #endif

        #ifdef ENABLE_TIM2
        case TIM2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, en);
            switch(TIM2_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, en);
                    break;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, en);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, en);
                    break;
            }
            break;
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, en);
            switch(TIM3_REMAP){
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, en);
                    break;
                case 3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, en);
                    break;
            }
            break;
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, en);
            switch(TIM4_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_TIM4, en);
                    break;
            }
            break;
        #endif

        #ifdef ENABLE_TIM5
        case TIM5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, en);
            break;
        #endif

        #ifdef ENABLE_TIM8
        case TIM8_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, en);
            switch(TIM8_REMAP){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_TIM8, en);
                    break;
            }
            break;
        #endif
    }
}

uint BasicTimer::getClk(){
    return isAdvancedTimer(instance) ? Sys::Clock::getAPB2Freq() : Sys::Clock::getAPB1Freq();
}


void BasicTimer::init(const uint32_t freq, const Mode mode, const bool en){
    this->enableRcc(true);
    uint32_t raw_period = this->getClk() / freq;

    // TIM_Get_BusFreq(instance);
    // uint32_t raw_period = 144000000 / freq;

    // can`t adaptly select the best frequency by gcd
    // TODO

    uint16_t cycle = 1;
    while(raw_period > 16384 * cycle){
        cycle++;
    }

    if(raw_period / cycle == 0) HALT;

    init(raw_period / cycle, cycle, mode, en);
}

void BasicTimer::init(const uint16_t period, const uint16_t cycle, const Mode mode, const bool en){
    this->enableRcc(true);

    TIM_InternalClockConfig(instance);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
        .TIM_Prescaler = uint16_t(MAX(int(cycle - 1), 0)),
        .TIM_CounterMode = (uint16_t)mode,
        .TIM_Period = uint16_t(MAX(int(period - 1), 0)),
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0,
    };

    TIM_TimeBaseInit(instance, &TIM_TimeBaseStructure);

    //令人困惑的是 删除这行将无法正常工作
    this->getClk();

    TIM_ClearFlag(instance, 0x1e7f);
    TIM_ClearITPendingBit(instance, 0x00ff);
    enable(en);
}


void BasicTimer::enable(const bool en){
    if(en){
        TIM_Cmd(instance, en);
        if(isAdvancedTimer(instance)){
            TIM_CtrlPWMOutputs(instance, en);
        }
    }else{
        TIM_Cmd(instance, DISABLE);
    }
}

void GenericTimer::initAsEncoder(const Mode mode){
    this->enableRcc(true);

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

	TIM_EncoderInterfaceConfig(instance,
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    TIM_Cmd(instance, ENABLE);
}

void GenericTimer::enableSingle(const bool _single){
    TIM_SelectOnePulseMode(instance, _single ? TIM_OPMode_Repetitive : TIM_OPMode_Single);
}

void GenericTimer::setTrgoSource(const TrgoSource source){
    TIM_SelectOutputTrigger(instance, (uint8_t)source);
}

void AdvancedTimer::initBdtr(const uint32_t ns, const LockLevel level){

    const TIM_BDTRInitTypeDef TIM_BDTRInitStructure{
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

        dead = MIN(dead, 254) >> 1;
        dead -= 64;
        dead &= mask;
        dead |= head;
    }else if(dead < 509){
        uint8_t head = 0b11000000;
        uint8_t mask = 0b00011111;

        dead = MIN(dead, 504) >> 1;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else if(dead < 1009){
        uint8_t head = 0b11100000;
        uint8_t mask = 0b00011111;

        dead = MIN(dead, 1008) >> 4;
        dead -= 32;
        dead &= mask;
        dead |= head;
    }else{
        dead = 0xff;
    }

    return dead;
}

void BasicTimer::enableIt(const IT it,const NvicPriority request, const bool en){
    NvicPriority::enable(request, ItToIrq(instance, it), en);
    TIM_ITConfig(instance, (uint16_t)it, (FunctionalState)en);
}


TimerOC & GenericTimer::oc(const size_t index){
    TIM_ASSERT(index <= 4 and index != 0);

    return channels[index - 1];
}


TimerChannel & GenericTimer::operator [](const int index){
    return channels[index];
}


TimerChannel & AdvancedTimer::operator [](const int index){
    TIM_ASSERT(index <= 4 and index >= -3);
    
    bool is_co = index < 0;
    if(is_co){
        return n_channels[-index - 1];
    }else{
        return channels[(index -1) & 0b11];
    }
}


TimerChannel & AdvancedTimer::operator [](const TimerChannel::ChannelIndex ch){
    bool is_co = (uint8_t) ch & 0b1;
    if(is_co){
        return n_channels[((uint8_t)ch - 1) >> 1];
    }else{
        return channels[(uint8_t)ch >> 1];
    }
}

void BasicTimer::handleIt(const IT it){
    const uint16_t code = uint8_t(it);
    auto & cb = this->cbs[CTZ(code)];
    EXECUTE(cb);
    TIM_ClearITPendingBit(instance, code);
}

#define HANDLE_IT(it)     if((itstatus & uint8_t(it)) and (itenable & uint8_t(it))) handleIt(it);


void BasicTimer::onUpdateHandler(){
    handleIt(IT::Update);
}

void BasicTimer::onBreakHandler(){
    handleIt(IT::Break);
}


void BasicTimer::onTriggerComHandler(){
    const uint16_t itstatus = instance->INTFR;
    const uint16_t itenable = instance->DMAINTENR;

    HANDLE_IT(IT::Trigger);
    HANDLE_IT(IT::COM);
}

void GenericTimer::onCCHandler(){
    const uint16_t itstatus = instance->INTFR;
    const uint16_t itenable = instance->DMAINTENR;

    HANDLE_IT(IT::CC1);
    HANDLE_IT(IT::CC2);
    HANDLE_IT(IT::CC3);
    HANDLE_IT(IT::CC4);
}

void GenericTimer::onItHandler(){
    const uint16_t itstatus = instance->INTFR;
    const uint16_t itenable = instance->DMAINTENR;

    HANDLE_IT(IT::Update);
    HANDLE_IT(IT::COM);
    HANDLE_IT(IT::Trigger);
    HANDLE_IT(IT::Break);
}

// void GenericTimer::onItHandler(){
//     const uint16_t itstatus = instance->INTFR;
//     const uint16_t itenable = instance->DMAINTENR;

//     HANDLE_IT(IT::Update);

//     HANDLE_IT(IT::CC1);
//     HANDLE_IT(IT::CC2);
//     HANDLE_IT(IT::CC3);
//     HANDLE_IT(IT::CC4);

//     HANDLE_IT(IT::COM);
//     HANDLE_IT(IT::Trigger);
//     HANDLE_IT(IT::Break);
// }