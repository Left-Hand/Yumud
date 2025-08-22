#pragma once

#include "core/clock/clock.hpp"
#include "timer_oc.hpp"
#include "timer_utils.hpp"
#include "hal/nvic/nvic.hpp"

#ifdef HDW_SXX32

#define ADVANCED_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void);\
__interrupt void TIM##x##_UP_IRQHandler(void);\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void);\
__interrupt void TIM##x##_CC_IRQHandler(void);\
}\


#define GENERIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#define BASIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\



#define DEF_ADVANCED_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_BRK_IRQHandler(void);\
friend void ::TIM##x##_UP_IRQHandler(void);\
friend void ::TIM##x##_TRG_COM_IRQHandler(void);\
friend void ::TIM##x##_CC_IRQHandler(void);\


#define DEF_GENERIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


#define DEF_BASIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


#ifdef ENABLE_TIM1
ADVANCED_TIMER_IT_FORWARD_DECL(1)
#endif

#ifdef ENABLE_TIM2
GENERIC_TIMER_IT_FORWARD_DECL(2)
#endif

#ifdef ENABLE_TIM3
GENERIC_TIMER_IT_FORWARD_DECL(3)
#endif

#ifdef ENABLE_TIM4
GENERIC_TIMER_IT_FORWARD_DECL(4)
#endif

#ifdef ENABLE_TIM5
GENERIC_TIMER_IT_FORWARD_DECL(5)
#endif

#ifdef ENABLE_TIM6
BASIC_TIMER_IT_FORWARD_DECL(6)
#endif

#ifdef ENABLE_TIM7
BASIC_TIMER_IT_FORWARD_DECL(7)
#endif

#ifdef ENABLE_TIM8
ADVANCED_TIMER_IT_FORWARD_DECL(8)
#endif

#ifdef ENABLE_TIM9
ADVANCED_TIMER_IT_FORWARD_DECL(9)
#endif

#ifdef ENABLE_TIM10
ADVANCED_TIMER_IT_FORWARD_DECL(10)
#endif

#undef ADVANCED_TIMER_IT_FORWARD_DECL
#undef GENERIC_TIMER_IT_FORWARD_DECL
#undef BASIC_TIMER_IT_FORWARD_DECL


namespace ymd::hal{

class BasicTimer{
public:
    using IT = TimerIT;
    using Mode = TimerCountMode;
    using TrgoSource = TimerTrgoSource;
    using Callback = std::function<void(void)>;
private:
    std::array<Callback, 8> cbs_;
protected:
    TIM_TypeDef * inst_;

    uint32_t get_bus_freq();
    void enable_rcc(const Enable en);

    
    __fast_inline Callback & get_callback(const IT it){
        switch(it){
            default:
            case IT::Update: return cbs_[0];
            case IT::CC1: return cbs_[1];
            case IT::CC2: return cbs_[2];
            case IT::CC3: return cbs_[3];
            case IT::CC4: return cbs_[4];
            case IT::COM: return cbs_[5];
            case IT::Trigger: return cbs_[6];
            case IT::Break: return cbs_[7];
        }
    }

    __fast_inline void invoke_callback(const IT it){
        auto & cb = get_callback(it);
        EXECUTE(cb);
    }

public:
    BasicTimer(TIM_TypeDef * _base):inst_(_base){;}

    struct Config{
        const uint32_t freq;
        const Mode mode = Mode::Up;
        const Enable en = EN;
    };

    void init(const Config & cfg);
    void remap(const uint8_t rm);
    void deinit();

    void enable(const Enable en = EN);
    void set_count_mode(const TimerCountMode mode);

    void set_psc(const uint16_t psc);
    void set_arr(const uint16_t arr);

    void set_freq(const uint32_t freq);

    void enable_it(const IT it,const NvicPriority request, const Enable en = EN);
    void enable_arr_sync(const Enable en = EN);
    void enable_psc_sync(const Enable en = EN);
    void enable_cc_ctrl_sync(const Enable en = EN);
    auto & inst() {return inst_;}

    volatile uint16_t & cnt(){return inst_->CNT;}
    volatile uint16_t & arr(){return inst_->ATRLR;}

    template<typename Fn>
    void attach(
            const IT it, 
            const NvicPriority & priority, 
            Fn && cb, const Enable en = EN){
        bind_cb(it, std::forward<Fn>(cb));
        enable_it(it, priority, en);
    }

    void attach(const IT it, const NvicPriority & priority, std::nullptr_t cb){
        attach(it, priority, nullptr, DISEN);
    }

    template<typename Fn>
    void bind_cb(const IT ch, Fn && cb){
        get_callback(ch) = std::forward<Fn>(cb);
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

    void init_as_encoder(const Mode mode = Mode::Up);
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

    __fast_inline void on_update_interrupt(){invoke_callback(IT::Update);}
    __fast_inline void on_break_interrupt(){invoke_callback(IT::Break);}
    __fast_inline void on_trigger_interrupt(){invoke_callback(IT::Trigger);}
    __fast_inline void on_com_interrupt(){invoke_callback(IT::COM);}

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

#ifdef ENABLE_TIM1
extern AdvancedTimer timer1;
#endif

#ifdef ENABLE_TIM2
extern GenericTimer timer2;
#endif

#ifdef ENABLE_TIM3
extern GenericTimer timer3;
#endif

#ifdef ENABLE_TIM4
extern GenericTimer timer4;
#endif

#ifdef ENABLE_TIM5
extern GenericTimer timer5;
#endif

#ifdef ENABLE_TIM6
extern BasicTimer timer6;
#endif

#ifdef ENABLE_TIM7
extern BasicTimer timer7;
#endif

#ifdef ENABLE_TIM8
extern AdvancedTimer timer8;
#endif

#ifdef ENABLE_TIM9
extern AdvancedTimer timer9;
#endif

#ifdef ENABLE_TIM10
extern AdvancedTimer timer10;
#endif

}

#endif
