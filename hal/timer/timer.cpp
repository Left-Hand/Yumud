#include "timer.hpp"
#include "core/system.hpp"
#include <optional>
#include "core/clock/clock.hpp"
#include "core/utils/Match.hpp"

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
using namespace ymd::hal;
using namespace ymd::fp;


#ifdef TIM_DEBUG
__inline void TIM_ASSERT(bool x){
    if(!x) HALT;
}

#else
#define TIM_ASSERT(x)
#endif


static std::tuple<uint16_t, uint16_t> dump_arr_and_psc(const TimerCountFreq count_freq, const uint32_t periph_freq){
    if(count_freq.is<NearestFreq>()){
        const auto arr_and_psc = ArrAndPsc::from_nearest_count_freq(
            periph_freq,
            count_freq.unwrap_as<NearestFreq>().count, 
            {0, 65535}
        );
        // PANIC(arr_and_psc.arr, arr_and_psc.psc);
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else if(count_freq.is<ArrAndPsc>()){
        const auto arr_and_psc = count_freq.unwrap_as<ArrAndPsc>();
        return std::make_tuple(arr_and_psc.arr, arr_and_psc.psc);
    }else{
        __builtin_trap();
    }
}

static constexpr uint8_t calculate_deadzone_code_from_ns(
    const uint32_t bus_freq, 
    const ymd::Nanoseconds ns
){

    const uint16_t scale = (ns.count() * (bus_freq / 1000000) / 1000);
    if(scale < 128){
        return scale;
    }else if(scale < 256){
        const uint8_t head = 0b10000000;
        const uint8_t mask = 0b00111111;

        return ((((MIN(scale, 254) >> 1) - 64) & mask) | head);
    }else if(scale < 509){
        const uint8_t head = 0b11000000;
        const uint8_t mask = 0b00011111;

        return ((((MIN(scale, 504) >> 1) - 32) & mask) | head);
    }else if(scale < 1009){
        const uint8_t head = 0b11100000;
        const uint8_t mask = 0b00011111;

        return (((MIN(scale, 1008) >> 4) - 32) & mask) | head;
    }else{
        return 0xff;
    }
}

void BasicTimer::enable_rcc(const Enable en){
    switch(reinterpret_cast<size_t>(inst_)){
        #ifdef ENABLE_TIM1
        case TIM1_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, en == EN);
            break;
        #endif

        #ifdef ENABLE_TIM2
        case TIM2_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, en == EN);
            break;
        #endif

        #ifdef ENABLE_TIM3
        case TIM3_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, en == EN);
            break;
        #endif

        #ifdef ENABLE_TIM4
        case TIM4_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, en == EN);
            break;
        #endif

        #ifdef ENABLE_TIM5
        case TIM5_BASE:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, en == EN);
            break;
        #endif

        #ifdef ENABLE_TIM8
        case TIM8_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, en == EN);
        #endif
    }
}

void BasicTimer::set_remap(const uint8_t rm){
    switch(reinterpret_cast<size_t>(inst_)){
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

uint32_t BasicTimer::get_bus_freq(){
    return details::is_advanced_timer(inst_) ? 
        sys::clock::get_apb2_clk_freq() : 
        sys::clock::get_apb1_clk_freq();
}

void BasicTimer::set_psc(const uint16_t psc){
    inst_->PSC = psc;
}
void BasicTimer::set_arr(const uint16_t arr){
    inst_->ATRLR = arr;
}

void BasicTimer::set_count_mode(const TimerCountMode mode){
    auto tmpcr1 = inst_->CTLR1;

    tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_DIR | TIM_CMS)));
    tmpcr1 |= (uint32_t)mode;

    tmpcr1 &= (uint16_t)(~((uint16_t)TIM_CTLR1_CKD));
    tmpcr1 |= (uint32_t)TIM_CKD_DIV1;

    inst_->CTLR1 = tmpcr1;
}

void BasicTimer::enable_arr_sync(const Enable en){
    if(en == EN){
        inst_->CTLR1 = inst_->CTLR1 | TIM_ARPE;
    }else{
        inst_->CTLR1 = (inst_->CTLR1) & uint16_t( ~((uint16_t)TIM_ARPE));
    }
}

void BasicTimer::enable_psc_sync(const Enable en){
    if(en == EN){
        inst_->SWEVGR = inst_->SWEVGR | TIM_PSCReloadMode_Immediate;
    }else{
        inst_->SWEVGR = inst_->SWEVGR & uint16_t( ~((uint16_t)TIM_PSCReloadMode_Immediate));
    }
}

void BasicTimer::set_freq(const TimerCountFreq count_freq){

    const auto [arr, psc] = dump_arr_and_psc(count_freq, get_bus_freq());
    set_arr(arr);
    set_psc(psc);
}


void BasicTimer::init(const Config & cfg, const Enable en){
    this->enable_rcc(EN);


    TIM_InternalClockConfig(inst_);
    const auto [arr, psc] = dump_arr_and_psc(cfg.count_freq, get_bus_freq());


    set_arr(arr);
    set_psc(psc);

    set_count_mode(cfg.count_mode);
    enable_arr_sync(EN);

    TIM_ClearFlag(inst_, 0x1e7f);
    TIM_ClearITPendingBit(inst_, 0x00ff);
    enable(en);
}


void BasicTimer::deinit(){
    this->enable_rcc(DISEN);
    callback_ = nullptr;
}


void BasicTimer::enable(const Enable en){
    TIM_Cmd(inst_, en == EN);
    
    if((en == EN) and details::is_advanced_timer(inst_)){
        TIM_CtrlPWMOutputs(inst_, en == EN);
    }
}

void GenericTimer::init_as_encoder(const CountMode mode){
    this->enable_rcc(EN);

    {
        const TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
            .TIM_Prescaler = 0,
            .TIM_CounterMode = (uint16_t)mode,
            .TIM_Period = 65535,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0,
        };

        TIM_TimeBaseInit(inst_, &TIM_TimeBaseStructure);
    }


    {
        TIM_ICInitTypeDef TIM_ICInitStruct = {
            .TIM_Channel = TIM_Channel_1,
            .TIM_ICPolarity = TIM_ICPolarity_Rising,
            .TIM_ICSelection = TIM_ICSelection_DirectTI,
            .TIM_ICPrescaler = TIM_ICPSC_DIV1,
            .TIM_ICFilter = 0x0F
        };

        TIM_ICInit(inst_,&TIM_ICInitStruct);

        TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
        TIM_ICInit(inst_,&TIM_ICInitStruct);
    }

	TIM_EncoderInterfaceConfig(inst_,
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    TIM_Cmd(inst_, ENABLE);
}

void GenericTimer::enable_single(const Enable en){
    TIM_SelectOnePulseMode(inst_, (en == EN) ? TIM_OPMode_Repetitive : TIM_OPMode_Single);
}

void GenericTimer::set_trgo_source(const TrgoSource source){
    TIM_SelectOutputTrigger(inst_, uint8_t(source));
}

void AdvancedTimer::init_bdtr(const Nanoseconds ns, const LockLevel level){

    const TIM_BDTRInitTypeDef TIM_BDTRInitStructure{
        .TIM_OSSRState = TIM_OSSRState_Disable,
        .TIM_OSSIState = TIM_OSSIState_Disable,
        .TIM_LOCKLevel = (uint16_t)level,
        .TIM_DeadTime = this->calculate_deadzone(ns),
        .TIM_Break = TIM_Break_Disable,
        .TIM_BreakPolarity = TIM_BreakPolarity_Low,
        .TIM_AutomaticOutput = TIM_AutomaticOutput_Enable
    };

    TIM_BDTRConfig(inst_, &TIM_BDTRInitStructure);
}

void AdvancedTimer::set_deadzone_ns(const Nanoseconds ns){
    uint8_t dead = this->calculate_deadzone(ns);

    uint16_t tempreg = inst_->BDTR;
    tempreg &= 0xff00;
    tempreg |= dead;
    inst_->BDTR = tempreg;
}

uint8_t AdvancedTimer::calculate_deadzone(const Nanoseconds ns){
    return calculate_deadzone_code_from_ns(
        this->get_bus_freq(),
        ns
    );
}


void BasicTimer::enable_cc_ctrl_sync(const Enable en){
    TIM_CCPreloadControl(inst_, en == EN);
}


#define TRY_HANDLE_IT(I)\
if((itstatus & uint8_t(I))) {\
    invoke_callback<I>(); \
    TIM_ClearITPendingBit(inst_, uint8_t(I)); \
    return;\
}\

void GenericTimer::on_cc_interrupt(){
    const uint16_t itstatus = inst_->INTFR;

    TRY_HANDLE_IT(IT::CC1);
    TRY_HANDLE_IT(IT::CC2);
    TRY_HANDLE_IT(IT::CC3);
    TRY_HANDLE_IT(IT::CC4);
}

void GenericTimer::on_it_interrupt(){
    const uint16_t itstatus = inst_->INTFR;

    TRY_HANDLE_IT(IT::Update);
    TRY_HANDLE_IT(IT::CC1);
    TRY_HANDLE_IT(IT::CC2);
    TRY_HANDLE_IT(IT::CC3);
    TRY_HANDLE_IT(IT::CC4);
    TRY_HANDLE_IT(IT::COM);
    TRY_HANDLE_IT(IT::Trigger);
    TRY_HANDLE_IT(IT::Break);
}

namespace ymd::hal{
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