#pragma once

#include "core/clock/clock.hpp"
#include "timer_oc.hpp"
#include "timer_utils.hpp"
#include "hal/nvic/nvic.hpp"

#ifdef HDW_SXX32

#define DEF_ADVANCED_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void);\
__interrupt void TIM##x##_UP_IRQHandler(void);\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void);\
__interrupt void TIM##x##_CC_IRQHandler(void);\
}\


#define DEF_GENERIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#define DEF_BASIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#ifdef ENABLE_TIM1
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(1)
#endif

#ifdef ENABLE_TIM2
DEF_GENERIC_TIMER_IT_FORWARD_DECL(2)
#endif

#ifdef ENABLE_TIM3
DEF_GENERIC_TIMER_IT_FORWARD_DECL(3)
#endif

#ifdef ENABLE_TIM4
DEF_GENERIC_TIMER_IT_FORWARD_DECL(4)
#endif

#ifdef ENABLE_TIM5
DEF_GENERIC_TIMER_IT_FORWARD_DECL(5)
#endif

#ifdef ENABLE_TIM6
DEF_BASIC_TIMER_IT_FORWARD_DECL(6)
#endif

#ifdef ENABLE_TIM7
DEF_BASIC_TIMER_IT_FORWARD_DECL(7)
#endif

#ifdef ENABLE_TIM8
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(8)
#endif

#ifdef ENABLE_TIM9
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(9)
#endif

#ifdef ENABLE_TIM10
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(10)
#endif

#undef DEF_ADVANCED_TIMER_IT_FORWARD_DECL
#undef DEF_GENERIC_TIMER_IT_FORWARD_DECL
#undef DEF_BASIC_TIMER_IT_FORWARD_DECL



#define DEF_ADVANCED_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_BRK_IRQHandler(void);\
friend void ::TIM##x##_UP_IRQHandler(void);\
friend void ::TIM##x##_TRG_COM_IRQHandler(void);\
friend void ::TIM##x##_CC_IRQHandler(void);\


#define DEF_GENERIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


#define DEF_BASIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\

namespace ymd::hal{

class BasicTimer{
public:
    using IT = TimerIT;
    using CountMode = TimerCountMode;
    using TrgoSource = TimerTrgoSource;
    using Callback = std::function<void(void)>;
private:
    std::array<Callback, 8> cbs_;
protected:
    TIM_TypeDef * inst_;

    uint32_t get_bus_freq();
    void enable_rcc(const Enable en);

    template<IT I>
    __fast_inline Callback & get_callback(){
        if constexpr (I == IT::Update)          return cbs_[0];
        else if constexpr (I == IT::CC1)        return cbs_[1];
        else if constexpr (I == IT::CC2)        return cbs_[2];
        else if constexpr (I == IT::CC3)        return cbs_[3];
        else if constexpr (I == IT::CC4)        return cbs_[4];
        else if constexpr (I == IT::COM)        return cbs_[5];
        else if constexpr (I == IT::Trigger)    return cbs_[6];
        else if constexpr (I == IT::Break)      return cbs_[7];
    }

    template<IT I>
    __fast_inline void invoke_callback(){
        auto & cb = get_callback<I>();
        EXECUTE(cb);
    }

public:
    BasicTimer(TIM_TypeDef * _base):inst_(_base){;}

    struct Config{
        const uint32_t freq;
        const CountMode count_mode = CountMode::Up;
    };

    void init(const Config & cfg, const Enable en);
    void set_remap(const uint8_t rm);
    void deinit();

    void enable(const Enable en);
    void set_count_mode(const TimerCountMode mode);

    void set_psc(const uint16_t psc);
    void set_arr(const uint16_t arr);

    void set_freq(const uint32_t freq);

    template<IT I>
    void enable_interrupt(const Enable en){
        TIM_ITConfig(inst_, std::bit_cast<uint8_t>(I), en == EN);
    }

    template<IT I>
    void register_nvic(const NvicPriority request, const Enable en){
        NvicPriority::enable(request, details::it_to_irq(inst_, I), en);
    }

    void enable_arr_sync(const Enable en);
    void enable_psc_sync(const Enable en);
    void enable_cc_ctrl_sync(const Enable en);
    auto & inst() {return inst_;}

    volatile uint16_t & cnt(){return inst_->CNT;}
    volatile uint16_t & arr(){return inst_->ATRLR;}

    template<IT I, typename Fn>
    void attach(
            const NvicPriority & priority, 
            Fn && cb, const Enable en){
        register_nvic<I>(priority, en);
        enable_interrupt<I>(en);
        set_interrupt_callback<I>(std::forward<Fn>(cb));
    }

    template<IT I>
    void attach(const NvicPriority & priority, std::nullptr_t cb){
        attach<I>(priority, nullptr, DISEN);
    }

    template<IT I, typename Fn>
    void set_interrupt_callback(Fn && cb){
        get_callback<I>() = std::forward<Fn>(cb);
    }

    #ifdef ENABLE_TIM6
    DEF_BASIC_TIMER_FRIEND_DECL(6)
    #endif

    #ifdef ENABLE_TIM7
    DEF_BASIC_TIMER_FRIEND_DECL(7)
    #endif
};

class GenericTimer:public BasicTimer{
protected:
    TimerOC channels[4];

    void on_cc_interrupt();
private:
    void on_it_interrupt();
public:
    GenericTimer(TIM_TypeDef * _base):
        BasicTimer(_base),
        channels{
            TimerOC(inst_, TimerChannel::ChannelNth::CH1),
            TimerOC(inst_, TimerChannel::ChannelNth::CH2),
            TimerOC(inst_, TimerChannel::ChannelNth::CH3),
            TimerOC(inst_, TimerChannel::ChannelNth::CH4)
        }{;}

    void init_as_encoder(const CountMode mode = CountMode::Up);
    void enable_single(const Enable en);
    void set_trgo_source(const TrgoSource source);

    template<size_t I>
    requires(I >= 1 and I <= 4)
    volatile uint16_t & cvr(){
        switch(I){
            case 1: return inst_->CH1CVR;
            case 2: return inst_->CH2CVR;
            case 3: return inst_->CH3CVR;
            case 4: return inst_->CH4CVR;
            default: __builtin_unreachable();
        }
    }

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOC & oc(){
        return channels[I - 1];
    }

    #ifdef ENABLE_TIM2
    DEF_GENERIC_TIMER_FRIEND_DECL(2)
    #endif

    #ifdef ENABLE_TIM3
    DEF_GENERIC_TIMER_FRIEND_DECL(3)
    #endif

    #ifdef ENABLE_TIM4
    DEF_GENERIC_TIMER_FRIEND_DECL(4)
    #endif

    #ifdef ENABLE_TIM5
    DEF_GENERIC_TIMER_FRIEND_DECL(5)
    #endif


};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t calculate_deadzone(const Nanoseconds deadzone_ns);

    TimerOCN n_channels[3];

public:
    using LockLevel = TimerBdtrLockLevel;

    AdvancedTimer(TIM_TypeDef * _base):
            GenericTimer(_base),
            n_channels{
                TimerOCN(inst_, TimerChannel::ChannelNth::CH1N),
                TimerOCN(inst_, TimerChannel::ChannelNth::CH2N),
                TimerOCN(inst_, TimerChannel::ChannelNth::CH3N),
            }{;}

    void init_bdtr(const Nanoseconds ns, const LockLevel level = LockLevel::Off);

    void set_deadzone_ns(const Nanoseconds ns);
    void set_repeat_times(const uint8_t rep){inst_->RPTCR = rep;}

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOCN & ocn(){return n_channels[I - 1];}

    #ifdef ENABLE_TIM1
    DEF_ADVANCED_TIMER_FRIEND_DECL(1);
    #endif

    #ifdef ENABLE_TIM8
    DEF_ADVANCED_TIMER_FRIEND_DECL(8);
    #endif

    #ifdef ENABLE_TIM9
    DEF_ADVANCED_TIMER_FRIEND_DECL(9);
    #endif

    #ifdef ENABLE_TIM10
    DEF_ADVANCED_TIMER_FRIEND_DECL(10);
    #endif


};


#undef DEF_BASIC_TIMER_FRIEND_DECL
#undef DEF_GENERIC_TIMER_FRIEND_DECL
#undef DEF_ADVANCED_TIMER_FRIEND_DECL


}

#endif
