#pragma once

#include "timer_oc.hpp"
#include "timer_utils.hpp"
#include "hal/sysmisc/nvic/nvic.hpp"
#include "core/utils/result.hpp"


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


#ifdef TIM1_PRESENT
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(1)
#endif

#ifdef TIM2_PRESENT
DEF_GENERIC_TIMER_IT_FORWARD_DECL(2)
#endif

#ifdef TIM3_PRESENT
DEF_GENERIC_TIMER_IT_FORWARD_DECL(3)
#endif

#ifdef TIM4_PRESENT
DEF_GENERIC_TIMER_IT_FORWARD_DECL(4)
#endif

#ifdef TIM5_PRESENT
DEF_GENERIC_TIMER_IT_FORWARD_DECL(5)
#endif

#ifdef TIM6_PRESENT
DEF_BASIC_TIMER_IT_FORWARD_DECL(6)
#endif

#ifdef TIM7_PRESENT
DEF_BASIC_TIMER_IT_FORWARD_DECL(7)
#endif

#ifdef TIM8_PRESENT
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(8)
#endif

#ifdef TIM9_PRESENT
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(9)
#endif

#ifdef TIM10_PRESENT
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


struct [[nodiscard]] TimerBdtr final{
    void * inst_;
    uint32_t bus_freq;

    struct [[nodiscard]] Config{
        TimerDeadzone deadzone;
        TimerBdtrLockLevel level = TimerBdtrLockLevel::Off;
    };

    void init(const Config &config);
    // void set_deadzone(const Nanoseconds nanos){
    //     set_deadzone_code(TimerDeadzoneCode::from_ns(bus_freq, nanos));
    // }
    // void set_deadzone_code(const TimerDeadzoneCode code);
};


using TimerLibError = Infallible;

struct [[nodiscard]] TimerPinSetuper final{
    using Error = TimerLibError;
    using Next = void;

    explicit TimerPinSetuper(void * inst, const TimerRemap remap) : 
        inst_(inst),
        remap_(remap){}

    Result<Next, Error> alter_to_pins(const std::initializer_list<TimerChannelSelection> list);
    Next dont_alter_to_pins();
private:
    void * inst_;
    TimerRemap remap_;
};




class [[nodiscard]] BasicTimer{
public:
    using Error = TimerLibError;
    using IT = TimerIT;
    using CountMode = TimerCountMode;
    using TrgoSource = TimerTrgoSource;
    using Callback = std::function<void(TimerEvent)>;
private:
    Callback event_callback_ = nullptr;
    void enable(const Enable en);
public:
    explicit BasicTimer(void * inst):inst_(inst){;}

    struct [[nodiscard]] Config{
        TimerRemap remap;
        TimerCountFreq count_freq;
        const CountMode count_mode;
    };

    Result<TimerPinSetuper, Error> init(const Config & cfg);

    void deinit();



    //启动定时器
    void start();

    //停止定时器
    void stop();
    
    //设置psc（SXX32专有）
    void set_psc(const uint16_t psc);

    //设置arr（SXX32专有）
    void set_arr(const uint16_t arr);

    //设置ckd（SXX32专有）
    void set_ckd(const uint8_t ckd);

    //设置udis（SXX32专有）
    void enable_udis(const Enable en);
    
    //设置计数频率
    void set_count_freq(const TimerCountFreq freq);

    //设置计数模式
    void set_count_mode(const TimerCountMode mode);

    //使能中断
    template<IT I>
    void enable_interrupt(const Enable en){
        dyn_enable_interrupt(I, en);
    }

    //将中断优先级注册到NVIC
    template<IT I>
    void register_nvic(const NvicPriority priority, const Enable en){
        priority.with_irqn(timer::details::it_to_irq(inst_, I)).enable(en);
    }

    //使能ARR同步更新（shadow）
    void enable_arr_sync(const Enable en);

    //使能PSC同步更新（shadow）
    void enable_psc_sync(const Enable en);
    void enable_cc_ctrl_sync(const Enable en);

    volatile uint16_t & cnt();
    volatile uint16_t & arr();

    //设置事件处理函数
    template<typename Fn>
    void set_event_callback(Fn && cb){
        event_callback_ = std::forward<Fn>(cb);
    }

    void set_remap(const TimerRemap rm);
protected:
    void * inst_;

    [[nodiscard]] uint32_t get_periph_clk_freq();
    void enable_rcc(const Enable en);

    //处理中断响应
    void accept_interrupt(const IT I){
        if(event_callback_ == nullptr) [[unlikely]]
            return;
        event_callback_(I);
    }

    void dyn_enable_interrupt(IT I,Enable en);

    #ifdef TIM6_PRESENT
    DEF_BASIC_TIMER_FRIEND_DECL(6)
    #endif

    #ifdef TIM7_PRESENT
    DEF_BASIC_TIMER_FRIEND_DECL(7)
    #endif
};

class [[nodiscard]] GeneralTimer:public BasicTimer{
protected:
    TimerOC channels_[4];
private:
    void on_interrupt();
public:
    explicit GeneralTimer(void * inst):
        BasicTimer(inst),
        channels_{
            TimerOC(inst_, TimerChannel::ChannelSelection::CH1),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH2),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH3),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH4)
        }{;}

    void init_as_encoder(const CountMode mode = CountMode::Up);
    void enable_single_shot(const Enable en);
    void set_trgo_source(const TimerTrgoSource source);
    void set_trgi_source(const TimerTrgiSource source);
    void set_slave_mode(const TimerSlaveMode mode);
    void enable_master_slave_mode(const Enable en);

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOC & oc(){
        return channels_[I - 1];
    }


    [[nodiscard]] bool is_up_counting();

    #ifdef TIM2_PRESENT
    DEF_GENERIC_TIMER_FRIEND_DECL(2)
    #endif

    #ifdef TIM3_PRESENT
    DEF_GENERIC_TIMER_FRIEND_DECL(3)
    #endif

    #ifdef TIM4_PRESENT
    DEF_GENERIC_TIMER_FRIEND_DECL(4)
    #endif

    #ifdef TIM5_PRESENT
    DEF_GENERIC_TIMER_FRIEND_DECL(5)
    #endif
};



class [[nodiscard]] AdvancedTimer:public GeneralTimer{
protected:
    TimerOCN n_channels_[3];
public:
    using LockLevel = TimerBdtrLockLevel;

    explicit AdvancedTimer(void * inst):
            GeneralTimer(inst),
            n_channels_{
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH1N),
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH2N),
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH3N),
            }{;}

    void on_cc_interrupt();
    TimerBdtr bdtr(){return TimerBdtr{
        .inst_ = inst_,
        .bus_freq = this->get_periph_clk_freq()
    };}
    void set_repeat_times(const uint8_t rep);

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOCN & ocn(){return n_channels_[I - 1];}


    #ifdef TIM1_PRESENT
    DEF_ADVANCED_TIMER_FRIEND_DECL(1);
    #endif

    #ifdef TIM8_PRESENT
    DEF_ADVANCED_TIMER_FRIEND_DECL(8);
    #endif

    #ifdef TIM9_PRESENT
    DEF_ADVANCED_TIMER_FRIEND_DECL(9);
    #endif

    #ifdef TIM10_PRESENT
    DEF_ADVANCED_TIMER_FRIEND_DECL(10);
    #endif


};


#undef DEF_BASIC_TIMER_FRIEND_DECL
#undef DEF_GENERIC_TIMER_FRIEND_DECL
#undef DEF_ADVANCED_TIMER_FRIEND_DECL


}

#endif
