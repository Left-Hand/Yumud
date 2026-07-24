#include "timer.hpp"
#include "core/sdk.hpp"
#include "timer_layout.hpp"

// 多个定时器同步输出的主从配置示例
// https://www.stmcu.org.cn/module/forum/forum.php?mod=viewthread&tid=622883&highlight=%E5%90%8C%E6%AD%A5



using namespace ymd;
using namespace ymd::hal;
using namespace ymd::hal::timer;

template<typename T>
static void reg_set_or_clear_bit(volatile T & reg, const T mask, const bool en){
    if(en){
        reg = static_cast<T>(reg) | static_cast<T>(mask);
    }else{
        reg = static_cast<T>(reg) & static_cast<T>(~mask);
    }
}



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

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, TIM_TypeDef)>(x))
// #define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::)>(x))



template<TimerRemap REMAP>
[[maybe_unused]] static Gpio _timer_to_ch1_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch2_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch3_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch4_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch1n_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch2n_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_ch3n_pin(const Nth nth){
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
[[maybe_unused]] static Gpio _timer_to_bkin_pin(const Nth nth){
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
[[maybe_unused]] static Gpio tim_to_##name##_pin(const Nth nth, const TimerRemap remap){\
    switch(remap){\
        case TimerRemap::_0: return _timer_to_##name##_pin<TimerRemap::_0>(nth);\
        case TimerRemap::_1: return _timer_to_##name##_pin<TimerRemap::_1>(nth);\
        case TimerRemap::_2: return _timer_to_##name##_pin<TimerRemap::_2>(nth);\
        case TimerRemap::_3: return _timer_to_##name##_pin<TimerRemap::_3>(nth);\
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



volatile uint16_t & BasicTimer::cnt(){return SPL_INST(p_inst_)->CNT;}
volatile uint16_t & BasicTimer::arr(){return SPL_INST(p_inst_)->ATRLR;}


void timer_alter_to_pin(
    const Nth tim_nth,
    const TimerChannelSelection sel,
    const TimerRemap remap
){
    switch(sel.kind()){
        case ChannelSelection::CH1:
            tim_to_ch1_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH2:
            tim_to_ch2_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH3:
            tim_to_ch3_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH4:
            tim_to_ch4_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH1N:
            tim_to_ch1n_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH2N:
            tim_to_ch2n_pin(tim_nth, remap).afpp();
            break;
        case ChannelSelection::CH3N:
            tim_to_ch3n_pin(tim_nth, remap).afpp();
            break;
    }
}


Result<TimerPinSetuper::Next, TimerPinSetuper::Error> TimerPinSetuper::alter_to_pins(
    const std::initializer_list<ChannelSelection> list
){
    for(const auto & sel : list){
        timer_alter_to_pin(tim_nth_, sel, remap_);
    }
    return Ok();
}

TimerPinSetuper::Next TimerPinSetuper::dont_alter_to_pins(){
    return TimerPinSetuper::Next();
}

void BasicTimer::enable_rcc(const Enable en){
    lld::timer_enable_rcc(tim_nth_, en);
}

void BasicTimer::set_remap(const TimerRemap rm){
    lld::timer_set_remap(tim_nth_, rm);
}

void BasicTimer::start(){
    enable(EN);
}

void BasicTimer::stop(){
    enable(DISEN);
}

void BasicTimer::dyn_enable_interrupt(const IT I, const Enable en){
    TIM_ITConfig(SPL_INST(p_inst_), std::bit_cast<uint16_t>(I), (en == EN));
}

uint32_t BasicTimer::get_periph_clk_freq(){
    return lld::is_advanced_timer(tim_nth_) ? 
        sys::clock::get_apb2_clk_freq() : 
        sys::clock::get_apb1_clk_freq();
}

void BasicTimer::set_psc(const uint16_t psc){
    SPL_INST(p_inst_)->PSC = psc;
}

void BasicTimer::set_arr(const uint16_t arr){
    SPL_INST(p_inst_)->ATRLR = arr;
}

//ckd = 0 : 1分频
//ckd = 1 : 2分频
//ckd = 2 : 4分频
//ckd = 3 : 保留
void BasicTimer::set_ckd(const uint8_t ckd){
    if(ckd > 2) __builtin_trap();

    auto tmpcr1 = SPL_INST(p_inst_)->CTLR1;
    
    tmpcr1 &= (uint16_t)(~((uint16_t)TIM_CTLR1_CKD));
    tmpcr1 |= (static_cast<uint16_t>(ckd) << 8);

    SPL_INST(p_inst_)->CTLR1 = tmpcr1;
}


void BasicTimer::set_count_mode(const TimerCountMode mode){
    auto tmpcr1 = SPL_INST(p_inst_)->CTLR1;

    tmpcr1 &= (uint16_t)(~((uint16_t)(TIM_DIR | TIM_CMS)));
    tmpcr1 |= (mode.to_bits() << 4);


    SPL_INST(p_inst_)->CTLR1 = tmpcr1;
}


template<typename T>
[[nodiscard]] static bool reg_get_bit(const volatile T & reg, const T mask){
    return static_cast<T>(reg) & mask;
}


void BasicTimer::enable_arr_sync(const Enable en){
    reg_set_or_clear_bit(SPL_INST(p_inst_)->CTLR1, TIM_ARPE, en == EN);
}

void BasicTimer::enable_psc_sync(const Enable en){
    reg_set_or_clear_bit(SPL_INST(p_inst_)->SWEVGR, TIM_PSCReloadMode_Immediate, en == EN);
}

void BasicTimer::enable_udis(const Enable en){
    reg_set_or_clear_bit(SPL_INST(p_inst_)->CTLR1, TIM_UDIS, en == EN);
}

void BasicTimer::set_count_freq(const TimerCountFreq count_freq){

    const auto [arr, psc] = lld::timer_calc_arr_and_psc(
        get_periph_clk_freq(), count_freq);
    set_arr(arr);
    set_psc(psc);
}


Result<TimerPinSetuper, TimerLibError> BasicTimer::init(const Config & cfg){
    TIM_DeInit(SPL_INST(p_inst_));
    set_remap(cfg.remap);
    SPL_INST(p_inst_)->CNT = 0;
    this->enable_rcc(EN);

    TIM_InternalClockConfig(SPL_INST(p_inst_));
    set_count_freq(cfg.count_freq);
    set_count_mode(cfg.count_mode);

    //默认不使用分频
    set_ckd(0);
    enable_arr_sync(EN);

    TIM_ClearFlag(SPL_INST(p_inst_), 0x1e7f);
    TIM_ClearITPendingBit(SPL_INST(p_inst_), 0x00ff);


    return Ok(TimerPinSetuper{p_inst_, cfg.remap});
}


void BasicTimer::deinit(){
    this->enable_rcc(DISEN);
    event_callback_ = nullptr;
}


void BasicTimer::enable(const Enable en){
    TIM_Cmd(SPL_INST(p_inst_), (en == EN));
    
    if(lld::is_advanced_timer(tim_nth_)){
        TIM_CtrlPWMOutputs(SPL_INST(p_inst_), (en == EN));
    }
}

void GeneralTimer::init_as_encoder(){
    this->enable_rcc(EN);
    constexpr auto mode = CountMode(CountMode::Up);
    {
        const TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure{
            .TIM_Prescaler = TIM_CKD_DIV1,
            .TIM_CounterMode = static_cast<uint16_t>(mode.to_bits() << 4),
            .TIM_Period = 0xffff,
            .TIM_ClockDivision = TIM_CKD_DIV1,
            .TIM_RepetitionCounter = 0,
        };

        TIM_TimeBaseInit(SPL_INST(p_inst_), &TIM_TimeBaseStructure);
    }


    {
        TIM_ICInitTypeDef TIM_ICInitStruct = {
            .TIM_Channel = TIM_Channel_1,
            .TIM_ICPolarity = TIM_ICPolarity_Rising,
            .TIM_ICSelection = TIM_ICSelection_DirectTI,
            .TIM_ICPrescaler = TIM_ICPSC_DIV1,
            .TIM_ICFilter = 0x0F
        };

        TIM_ICInit(SPL_INST(p_inst_),&TIM_ICInitStruct);
    }

    {
        TIM_ICInitTypeDef TIM_ICInitStruct = {
            .TIM_Channel = TIM_Channel_2,
            .TIM_ICPolarity = TIM_ICPolarity_Rising,
            .TIM_ICSelection = TIM_ICSelection_DirectTI,
            .TIM_ICPrescaler = TIM_ICPSC_DIV1,
            .TIM_ICFilter = 0x0F
        };

        TIM_ICInit(SPL_INST(p_inst_),&TIM_ICInitStruct);
    }

	TIM_EncoderInterfaceConfig(SPL_INST(p_inst_),
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    TIM_Cmd(SPL_INST(p_inst_), ENABLE);
}


bool GeneralTimer::is_up_counting(){
    return lld::timer_is_up_counting(p_inst_);
}


void GeneralTimer::enable_single_shot(const Enable en){
    TIM_SelectOnePulseMode(SPL_INST(p_inst_), (en == EN) ? TIM_OPMode_Repetitive : TIM_OPMode_Single);
}

void GeneralTimer::set_trgo_source(const TrgoSource source){
    TIM_SelectOutputTrigger(SPL_INST(p_inst_), std::bit_cast<uint8_t>(source) << 4);
}

void GeneralTimer::set_trgi_source(const TrgiSource source){
    TIM_SelectInputTrigger(SPL_INST(p_inst_), std::bit_cast<uint8_t>(source) << 4);
}

void GeneralTimer::set_slave_mode(const SlaveMode slave_mode){
    TIM_SelectSlaveMode(SPL_INST(p_inst_), std::bit_cast<uint8_t>(slave_mode));
}

void GeneralTimer::enable_master_slave_mode(const Enable en){
    TIM_SelectMasterSlaveMode(SPL_INST(p_inst_), 
        (en == EN) ? TIM_MasterSlaveMode_Enable : TIM_MasterSlaveMode_Disable);
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
        if(cfg.deadzone.is<TimerDeadzoneCode>()) 
            return cfg.deadzone.unwrap_as<TimerDeadzoneCode>();
        if(cfg.deadzone.is<Nanoseconds>()) 
            return TimerDeadzoneCode::from_ns(self.bus_freq, cfg.deadzone.unwrap_as<Nanoseconds>());
        __builtin_trap();
    }();

    // https://zhuanlan.zhihu.com/p/648629584
    const TIM_BDTRInitTypeDef TIM_BDTRInitStructure{
        .TIM_OSSRState = TIM_OSSRState_Enable,
        .TIM_OSSIState = TIM_OSSIState_Enable,
        .TIM_LOCKLevel = static_cast<uint16_t>(std::bit_cast<uint8_t>(cfg.level) << 8),
        .TIM_DeadTime = deadzone_code.bits,
        .TIM_Break = TIM_Break_Disable,
        // .TIM_Break = TIM_Break_Enable,
        // .TIM_BreakPolarity = TIM_BreakPolarity_Low,
        .TIM_BreakPolarity = TIM_BreakPolarity_High,

        //当刹车信号恢复正常时，PWM信号是否会自动恢复
        .TIM_AutomaticOutput = TIM_AutomaticOutput_Enable
    };

    TIM_BDTRConfig(SPL_INST(p_inst_), &TIM_BDTRInitStructure);
}

#if 0
void TimerBdtr::set_deadzone_code(const TimerDeadzoneCode deadzone_code){
    uint16_t tempreg = SPL_INST(p_inst_)->BDTR;
    tempreg &= 0xff00;
    tempreg |= deadzone_code.bits;
    SPL_INST(p_inst_)->BDTR = tempreg;
}
#endif

void BasicTimer::enable_cc_ctrl_sync(const Enable en){
    TIM_CCPreloadControl(SPL_INST(p_inst_), (en == EN));
}


static constexpr uint16_t VALID_INTERRUPT_FLAG_MASK = 0x00ff;

#define TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, I)\
if((pending_intr_flag & static_cast<uint16_t>(I))) {\
    static constexpr uint16_t INVERTED_FLAG_MASK = ~(static_cast<uint16_t>(I) & VALID_INTERRUPT_FLAG_MASK);\
    invoke_callback(I); \
    SPL_INST(p_inst_)->INTFR = INVERTED_FLAG_MASK;\
    pending_intr_flag &= INVERTED_FLAG_MASK;\
    if(pending_intr_flag == 0) [[likely]] return;\
    return;\
}\

void AdvancedTimer::isr_cc(){
    uint16_t pending_intr_flag = 
        static_cast<uint16_t>(SPL_INST(p_inst_)->INTFR)
        & static_cast<uint16_t>(SPL_INST(p_inst_)->DMAINTENR) & VALID_INTERRUPT_FLAG_MASK
    ;

    if(pending_intr_flag == 0) [[unlikely]] return;

    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC1);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC2);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC3);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC4);
}

void BasicTimer::isr_common(){
    uint16_t pending_intr_flag = 
        static_cast<uint16_t>(SPL_INST(p_inst_)->INTFR)
        & static_cast<uint16_t>(SPL_INST(p_inst_)->DMAINTENR) & VALID_INTERRUPT_FLAG_MASK
    ;

    if(pending_intr_flag == 0) [[unlikely]] return;

    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::Update);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC1);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC2);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC3);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::CC4);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::COM);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::Trigger);
    TRY_HANDLE_FLAG_OR_RETURN(pending_intr_flag, IT::Break);
}

void AdvancedTimer::set_repeat_times(const uint16_t rep){
    SPL_INST(p_inst_)->RPTCR = rep;
}

