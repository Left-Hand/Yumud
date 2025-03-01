#include "timer.hpp"
#include "sys/core/system.hpp"

#define TIM1_RM_A8_A9_A10_A11__B13_B14_B15 0
#define TIM1_RM_A8_A9_A10_A11__A7_B0_B1 1
#define TIM1_RM_E9_E11_E13_E14__E8_E10_E12 3

#define TIM2_RM_A0_A1_A2_A3 0
#define TIM2_RM_A15_B3_A2_A3 1
#define TIM2_RM_A0_A1_B10_B11 2
#define TIM2_RM_A15_B3_B10_B11 3

#define TIM3_RM_A6_A7_B0_B1 0
#define TIM3_RM_B4_B5_B0_B1 2

#define TIM4_RM_B6_B7_B8_B9 0
#define TIM4_RM_D12_D13_D14_D15 1

#define TIM5_RM_A0_A1_A2_A3 0

#define TIM6_RM_A0_A1_A2_A3 0

#define TIM8_RM_C6_C7_C8_C9__A7_B0_B1 0
#define TIM8_RM_B6_B7_B8_C13__A13_A14_A15 1

#define TIM9_RM_A2_A3_A4_C4__C0_C1_C2 0
#define TIM9_RM_A2_A3_A4_C4__B0_B1_B2 1
#define TIM9_RM_D9_D11_D13_D15__D8_D10_D12 2

#define TIM10_RM_B8_B9_C3_C11__A12_A13_A14 0
#define TIM10_RM_B3_B4_B5_C14__A5_A6_A7 1
#define TIM10_RM_D1_D3_D5_D7__E3_E4_E5 2

using namespace ymd;
using namespace ymd::TimerUtils::internal;


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

void BasicTimer::remap(const uint8_t rm){
    switch(uint32_t(instance)){
        #ifdef ENABLE_TIM1
        case TIM1_BASE:
            switch(rm){
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

        #ifdef ENABLE_TIM2
        case TIM2_BASE:
            switch(rm){
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

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
            switch(rm){
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

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
            switch(rm){
                case 0:
                    break;
                case 1:
                    GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);
                    break;
            }
            break;
        #endif

        #ifdef ENABLE_TIM5
        case TIM5_BASE:
            //no remap
            break;
        #endif

        #ifdef ENABLE_TIM8
        case TIM8_BASE:
            switch(rm){
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
    return isAdvancedTimer(instance) ? sys::Clock::getAPB2Freq() : sys::Clock::getAPB1Freq();
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
    TIM_Cmd(instance, en);
    
    if(en and isAdvancedTimer(instance)){
        TIM_CtrlPWMOutputs(instance, en);
    }
}

void GenericTimer::initAsEncoder(const Mode mode){
    this->enableRcc(true);

    {
        const TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
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

void BasicTimer::invokeCallback(const IT it){
    const uint16_t code = uint8_t(it);
    auto & cb = this->cbs[CTZ(code)];
    EXECUTE(cb);
    TIM_ClearITPendingBit(instance, code);
}

#define TRY_HANDLE_IT(it)     if((itstatus & uint8_t(it))) {invokeCallback(it);return;}


void BasicTimer::onUpdateInterrupt(){
    invokeCallback(IT::Update);
}

void BasicTimer::onBreakInterrupt(){
    invokeCallback(IT::Break);
}


void BasicTimer::onTriggerComInterrupt(){
    const uint16_t itstatus = instance->INTFR;
    // const uint16_t itenable = instance->DMAINTENR;

    TRY_HANDLE_IT(IT::Trigger);
    TRY_HANDLE_IT(IT::COM);
}

void GenericTimer::onCCInterrupt(){
    const uint16_t itstatus = instance->INTFR;
    // const uint16_t itenable = instance->DMAINTENR;

    TRY_HANDLE_IT(IT::CC1);
    TRY_HANDLE_IT(IT::CC2);
    TRY_HANDLE_IT(IT::CC3);
    TRY_HANDLE_IT(IT::CC4);
}

void GenericTimer::onItInterrupt(){
    const uint16_t itstatus = instance->INTFR;
    // const uint16_t itenable = instance->DMAINTENR;

    TRY_HANDLE_IT(IT::Update);
    TRY_HANDLE_IT(IT::COM);
    TRY_HANDLE_IT(IT::Trigger);
    TRY_HANDLE_IT(IT::Break);
}

// void GenericTimer::onItInterrupt(){
//     const uint16_t itstatus = instance->INTFR;
//     const uint16_t itenable = instance->DMAINTENR;

//     TRY_HANDLE_IT(IT::Update);

//     TRY_HANDLE_IT(IT::CC1);
//     TRY_HANDLE_IT(IT::CC2);
//     TRY_HANDLE_IT(IT::CC3);
//     TRY_HANDLE_IT(IT::CC4);

//     TRY_HANDLE_IT(IT::COM);
//     TRY_HANDLE_IT(IT::Trigger);
//     TRY_HANDLE_IT(IT::Break);
// }


namespace ymd{
#ifdef ENABLE_TIM1
AdvancedTimer timer1{TIM1};
#endif

#ifdef ENABLE_TIM2
GenericTimer timer2{TIM2};
#endif

#ifdef ENABLE_TIM3
GenericTimer timer3{TIM3};
#endif

#ifdef ENABLE_TIM4
GenericTimer timer4{TIM4};
#endif

#ifdef ENABLE_TIM5
GenericTimer timer5{TIM5};
#endif

#ifdef ENABLE_TIM6
BasicTimer timer6{TIM6};
#endif

#ifdef ENABLE_TIM7
BasicTimer timer7{TIM7};
#endif

#ifdef ENABLE_TIM8
AdvancedTimer timer8{TIM8};
#endif

#ifdef ENABLE_TIM9
AdvancedTimer timer9{TIM9};
#endif

#ifdef ENABLE_TIM10
AdvancedTimer timer10{TIM10};
#endif
}