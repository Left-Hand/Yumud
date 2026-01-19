#include "timer.hpp"
#include "core/sdk.hpp"
#include "timer_layout.hpp"

// 多个定时器同步输出的主从配置示例
// https://www.stmcu.org.cn/module/forum/forum.php?mod=viewthread&tid=622883&highlight=%E5%90%8C%E6%AD%A5



using namespace ymd;
using namespace ymd::hal;
using namespace ymd::hal::timer;


#ifdef TIM_DEBUG
__inline void TIM_ASSERT(bool x){
    if(!x) HALT;
}

#else
#define TIM_ASSERT(x)
#endif

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))



[[maybe_unused]] static Nth _timer_to_nth(const void * inst){

    switch(reinterpret_cast<size_t>(inst)){
        #ifdef TIM1_PRESENT
        case TIM1_BASE: return Nth(1);
        #endif
        #ifdef TIM2_PRESENT
        case TIM2_BASE: return Nth(2);
        #endif
        #ifdef TIM3_PRESENT
        case TIM3_BASE: return Nth(3);
        #endif
        #ifdef TIM4_PRESENT
        case TIM4_BASE: return Nth(4);
        #endif
        #ifdef TIM5_PRESENT
        case TIM5_BASE: return Nth(5);
        #endif
        #ifdef TIM6_PRESENT
        case TIM6_BASE: return Nth(6);
        #endif
        #ifdef TIM7_PRESENT
        case TIM7_BASE: return Nth(7);
        #endif
        #ifdef TIM8_PRESENT
        case TIM8_BASE: return Nth(8);
        #endif
        #ifdef TIM9_PRESENT
        case TIM9_BASE: return Nth(9);
        #endif
        #ifdef TIM10_PRESENT
        case TIM10_BASE: return Nth(10);
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch1_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch1_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch1_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch1_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch1_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch1_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch1_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch1_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch1_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch2_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch2_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch2_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch2_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch2_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch2_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch2_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch2_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch2_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch3_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch3_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch3_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch3_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch3_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch3_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch3_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch3_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch3_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch4_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch4_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch4_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch4_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch4_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch4_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch4_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch4_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch4_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch1n_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch1n_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch1n_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch1n_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch1n_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch1n_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch1n_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch1n_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch1n_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch2n_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch2n_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch2n_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch2n_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch2n_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch2n_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch2n_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch2n_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch2n_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch3n_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::ch3n_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM2_PRESENT
        case 2:
            return pintag_to_pin<timer::ch3n_pin_t<2, REMAP>>();
        #endif
        #ifdef TIM3_PRESENT
        case 3:
            return pintag_to_pin<timer::ch3n_pin_t<3, REMAP>>();
        #endif
        #ifdef TIM4_PRESENT
        case 4:
            return pintag_to_pin<timer::ch3n_pin_t<4, REMAP>>();
        #endif
        #ifdef TIM5_PRESENT
        case 5:
            return pintag_to_pin<timer::ch3n_pin_t<5, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::ch3n_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::ch3n_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::ch3n_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_bkin_pin(const void * inst){
    const auto nth = _timer_to_nth(inst);
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1:
            return pintag_to_pin<timer::bkin_pin_t<1, REMAP>>();
        #endif
        #ifdef TIM8_PRESENT
        case 8:
            return pintag_to_pin<timer::bkin_pin_t<8, REMAP>>();
        #endif
        #ifdef TIM9_PRESENT
        case 9:
            return pintag_to_pin<timer::bkin_pin_t<9, REMAP>>();
        #endif
        #ifdef TIM10_PRESENT
        case 10:
            return pintag_to_pin<timer::bkin_pin_t<10, REMAP>>();
        #endif
    }
    __builtin_trap();
}

#define DEF_TIM_BIND_PIN_LAYOUTER(name)\
[[maybe_unused]] static Gpio tim_to_##name##_pin(const void * inst, const TimerRemap remap){\
    switch(remap){\
        case TimerRemap::_0: return _timer_to_##name##_pin<TimerRemap::_0>(inst);\
        case TimerRemap::_1: return _timer_to_##name##_pin<TimerRemap::_1>(inst);\
        case TimerRemap::_2: return _timer_to_##name##_pin<TimerRemap::_2>(inst);\
        case TimerRemap::_3: return _timer_to_##name##_pin<TimerRemap::_3>(inst);\
    }\
    __builtin_trap();\
}\

DEF_TIM_BIND_PIN_LAYOUTER(ch1)
DEF_TIM_BIND_PIN_LAYOUTER(ch2)
DEF_TIM_BIND_PIN_LAYOUTER(ch3)
DEF_TIM_BIND_PIN_LAYOUTER(ch4)
DEF_TIM_BIND_PIN_LAYOUTER(ch1n)
DEF_TIM_BIND_PIN_LAYOUTER(ch2n)
DEF_TIM_BIND_PIN_LAYOUTER(ch3n)
DEF_TIM_BIND_PIN_LAYOUTER(bkin)



volatile uint16_t & BasicTimer::cnt(){return SDK_INST(inst_)->CNT;}
volatile uint16_t & BasicTimer::arr(){return SDK_INST(inst_)->ATRLR;}


Result<TimerPinSetuper::Next, TimerPinSetuper::Error> TimerPinSetuper::alter_to_pins(
    const std::initializer_list<ChannelSelection> list
){
    for(const auto & sel : list){
        switch(sel.kind()){
        case ChannelSelection::CH1:
            tim_to_ch1_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH2:
            tim_to_ch2_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH3:
            tim_to_ch3_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH4:
            tim_to_ch4_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH1N:
            tim_to_ch1n_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH2N:
            tim_to_ch2n_pin(inst_, remap_).afpp();
            break;
        case ChannelSelection::CH3N:
            tim_to_ch3n_pin(inst_, remap_).afpp();
            break;
        }
    }
    return Ok();
}

TimerPinSetuper::Next TimerPinSetuper::dont_alter_to_pins(){
    return TimerPinSetuper::Next();
}

void BasicTimer::enable_rcc(const Enable en){
    timer::details::enable_rcc(SDK_INST(inst_), en);
}

void BasicTimer::set_remap(const TimerRemap rm){
    // PANIC{uint8_t(rm)};
    timer::details::set_remap(SDK_INST(inst_), rm);
}

void BasicTimer::start(){
    enable(EN);
}

void BasicTimer::stop(){
    enable(DISEN);
}

void BasicTimer::dyn_enable_interrupt(const IT I, const Enable en){
    TIM_ITConfig(SDK_INST(inst_), std::bit_cast<uint16_t>(I), en == EN);
}

uint32_t BasicTimer::get_periph_clk_freq(){
    return timer::details::is_advanced_timer(SDK_INST(inst_)) ? 
        sys::clock::get_apb2_clk_freq() : 
        sys::clock::get_apb1_clk_freq();
}

void BasicTimer::set_psc(const uint16_t psc){
    SDK_INST(inst_)->PSC = psc;
}

void BasicTimer::set_arr(const uint16_t arr){
    SDK_INST(inst_)->ATRLR = arr;
}

//ckd = 0 : 1分频
//ckd = 1 : 2分频
//ckd = 2 : 4分频
//ckd = 3 : 保留
void BasicTimer::set_ckd(const uint8_t ckd){
    if(ckd > 2) __builtin_trap();

    auto tmpcr1 = SDK_INST(inst_)->CTLR1;
    
    tmpcr1 &= (uint16_t)(~((uint16_t)TIM_CTLR1_CKD));
    tmpcr1 |= (static_cast<uint16_t>(ckd) << 8);

    SDK_INST(inst_)->CTLR1 = tmpcr1;
}


void BasicTimer::set_count_mode(const TimerCountMode mode){
    auto tmpcr1 = SDK_INST(inst_)->CTLR1;

    tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_DIR | TIM_CMS)));
    tmpcr1 |= (mode.to_bits() << 4);


    SDK_INST(inst_)->CTLR1 = tmpcr1;
}

void BasicTimer::enable_arr_sync(const Enable en){
    if(en == EN){
        SDK_INST(inst_)->CTLR1 = SDK_INST(inst_)->CTLR1 | TIM_ARPE;
    }else{
        SDK_INST(inst_)->CTLR1 = (SDK_INST(inst_)->CTLR1) & uint16_t( ~((uint16_t)TIM_ARPE));
    }
}

void BasicTimer::enable_psc_sync(const Enable en){
    if(en == EN){
        SDK_INST(inst_)->SWEVGR = SDK_INST(inst_)->SWEVGR | TIM_PSCReloadMode_Immediate;
    }else{
        SDK_INST(inst_)->SWEVGR = SDK_INST(inst_)->SWEVGR & uint16_t( ~((uint16_t)TIM_PSCReloadMode_Immediate));
    }
}

void BasicTimer::enable_udis(const Enable en){
    if(en == EN){
        SDK_INST(inst_)->CTLR1 |= TIM_UDIS;
    }else{
        SDK_INST(inst_)->CTLR1 &= ~TIM_UDIS;
    }
}

void BasicTimer::set_count_freq(const TimerCountFreq count_freq){

    const auto [arr, psc] = timer::details::calc_arr_and_psc(
        get_periph_clk_freq(), count_freq);
    set_arr(arr);
    set_psc(psc);
}


Result<TimerPinSetuper, TimerLibError> BasicTimer::init(const Config & cfg){
    TIM_DeInit(SDK_INST(inst_));
    set_remap(cfg.remap);
    SDK_INST(inst_)->CNT = 0;
    this->enable_rcc(EN);

    TIM_InternalClockConfig(SDK_INST(inst_));
    set_count_freq(cfg.count_freq);
    set_count_mode(cfg.count_mode);

    //默认不使用分频
    set_ckd(0);
    enable_arr_sync(EN);

    TIM_ClearFlag(SDK_INST(inst_), 0x1e7f);
    TIM_ClearITPendingBit(SDK_INST(inst_), 0x00ff);


    return Ok(TimerPinSetuper{inst_, cfg.remap});
}


void BasicTimer::deinit(){
    this->enable_rcc(DISEN);
    callback_ = nullptr;
}


void BasicTimer::enable(const Enable en){
    TIM_Cmd(SDK_INST(inst_), en == EN);
    
    if((en == EN) and timer::details::is_advanced_timer(SDK_INST(inst_))){
        TIM_CtrlPWMOutputs(SDK_INST(inst_), en == EN);
    }
}

void GeneralTimer::init_as_encoder(const CountMode mode){
    this->enable_rcc(EN);

    {
        const TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
            .TIM_Prescaler = 0,
            .TIM_CounterMode = static_cast<uint16_t>(mode.to_bits() << 4),
            .TIM_Period = 0xffff,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0,
        };

        TIM_TimeBaseInit(SDK_INST(inst_), &TIM_TimeBaseStructure);
    }


    {
        TIM_ICInitTypeDef TIM_ICInitStruct = {
            .TIM_Channel = TIM_Channel_1,
            .TIM_ICPolarity = TIM_ICPolarity_Rising,
            .TIM_ICSelection = TIM_ICSelection_DirectTI,
            .TIM_ICPrescaler = TIM_ICPSC_DIV1,
            .TIM_ICFilter = 0x0F
        };

        TIM_ICInit(SDK_INST(inst_),&TIM_ICInitStruct);

        TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
        TIM_ICInit(SDK_INST(inst_),&TIM_ICInitStruct);
    }

	TIM_EncoderInterfaceConfig(SDK_INST(inst_),
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    TIM_Cmd(SDK_INST(inst_), ENABLE);
}

bool GeneralTimer::is_up_counting(){
    auto tmpcr1 = SDK_INST(inst_)->CTLR1;
    return (tmpcr1 & TIM_DIR) == 0;
}


void GeneralTimer::enable_single_shot(const Enable en){
    TIM_SelectOnePulseMode(SDK_INST(inst_), (en == EN) ? TIM_OPMode_Repetitive : TIM_OPMode_Single);
}

void GeneralTimer::set_trgo_source(const TrgoSource source){
    TIM_SelectOutputTrigger(SDK_INST(inst_), std::bit_cast<uint8_t>(source) << 4);
}

void GeneralTimer::set_trgi_source(const TrgiSource source){
    TIM_SelectInputTrigger(SDK_INST(inst_), std::bit_cast<uint8_t>(source) << 4);
}

void GeneralTimer::set_slave_mode(const SlaveMode slave_mode){
    TIM_SelectSlaveMode(SDK_INST(inst_), std::bit_cast<uint8_t>(slave_mode));
}

void GeneralTimer::enable_master_slave_mode(const Enable en){
    TIM_SelectMasterSlaveMode(SDK_INST(inst_), 
        en == EN ? TIM_MasterSlaveMode_Enable : TIM_MasterSlaveMode_Disable);
}

void TimerBdtr::init(const Config & cfg){
    auto & self = *this;
    // MOE
    // 主输出使能位。一旦刹车信号有效，将被异步清零。
    // 1：允许 OCx 和 OCxN 设为输出；
    // 0：禁止 OCx 和 OCxN 的输出或者强制为空闲状态。


    // AOE
    // 自动输出使能。
    // 1：MOE 可以被软件置位或者在下一个更新事件中被
    // 置位；
    // 0：MOE 只能被软件置位。

    const auto deadzone_code = [&] -> TimerDeadzoneCode{
        if(cfg.deadzone.is<TimerDeadzoneCode>()) return cfg.deadzone.unwrap_as<TimerDeadzoneCode>();
        if(cfg.deadzone.is<Nanoseconds>()) return TimerDeadzoneCode::from_ns(self.bus_freq, cfg.deadzone.unwrap_as<Nanoseconds>());
        __builtin_trap();
    }();
    const TIM_BDTRInitTypeDef TIM_BDTRInitStructure{
        .TIM_OSSRState = TIM_OSSRState_Disable,
        .TIM_OSSIState = TIM_OSSIState_Disable,
        .TIM_LOCKLevel = static_cast<uint16_t>(std::bit_cast<uint8_t>(cfg.level) << 8),
        .TIM_DeadTime = deadzone_code.bits,
        .TIM_Break = TIM_Break_Disable,
        .TIM_BreakPolarity = TIM_BreakPolarity_Low,
        .TIM_AutomaticOutput = TIM_AutomaticOutput_Enable
    };

    TIM_BDTRConfig(SDK_INST(inst_), &TIM_BDTRInitStructure);
}

#if 0
void TimerBdtr::set_deadzone_code(const TimerDeadzoneCode deadzone_code){
    uint16_t tempreg = SDK_INST(inst_)->BDTR;
    tempreg &= 0xff00;
    tempreg |= deadzone_code.bits;
    SDK_INST(inst_)->BDTR = tempreg;
}
#endif

void BasicTimer::enable_cc_ctrl_sync(const Enable en){
    TIM_CCPreloadControl(SDK_INST(inst_), en == EN);
}


#define TRY_ACCEPT_AND_CLEAR_IT(I)\
if((itstatus & static_cast<uint16_t>(I))) {\
    accept_interrupt(I); \
    TIM_ClearITPendingBit(SDK_INST(inst_), uint8_t(I)); \
    return;\
}\

void AdvancedTimer::on_cc_interrupt(){
    const uint16_t itstatus = SDK_INST(inst_)->INTFR;

    TRY_ACCEPT_AND_CLEAR_IT(IT::CC1);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC2);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC3);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC4);
}

void GeneralTimer::on_interrupt(){
    const uint16_t itstatus = SDK_INST(inst_)->INTFR;

    TRY_ACCEPT_AND_CLEAR_IT(IT::Update);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC1);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC2);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC3);
    TRY_ACCEPT_AND_CLEAR_IT(IT::CC4);
    TRY_ACCEPT_AND_CLEAR_IT(IT::COM);
    TRY_ACCEPT_AND_CLEAR_IT(IT::Trigger);
    TRY_ACCEPT_AND_CLEAR_IT(IT::Break);
}

void AdvancedTimer::set_repeat_times(const uint8_t rep){
    SDK_INST(inst_)->RPTCR = rep;
}

namespace ymd::hal{
#ifdef TIM1_PRESENT
AdvancedTimer timer1{TIM1};
#endif

#ifdef TIM2_PRESENT
GeneralTimer timer2{TIM2};
#endif

#ifdef TIM3_PRESENT
GeneralTimer timer3{TIM3};
#endif

#ifdef TIM4_PRESENT
GeneralTimer timer4{TIM4};
#endif

#ifdef TIM5_PRESENT
GeneralTimer timer5{TIM5};
#endif

#ifdef TIM6_PRESENT
BasicTimer timer6{TIM6};
#endif

#ifdef TIM7_PRESENT
BasicTimer timer7{TIM7};
#endif

#ifdef TIM8_PRESENT
AdvancedTimer timer8{TIM8};
#endif

#ifdef TIM9_PRESENT
AdvancedTimer timer9{TIM9};
#endif

#ifdef TIM10_PRESENT
AdvancedTimer timer10{TIM10};
#endif
}