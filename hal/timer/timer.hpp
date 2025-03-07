#pragma once

#include "timer_oc.hpp"
#include "timer_utils.hpp"

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



#define ADVANCED_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_BRK_IRQHandler(void);\
friend void ::TIM##x##_UP_IRQHandler(void);\
friend void ::TIM##x##_TRG_COM_IRQHandler(void);\
friend void ::TIM##x##_CC_IRQHandler(void);\


#define GENERIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


#define BASIC_TIMER_FRIEND_DECL(x)\
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
class TimerHw{};

class BasicTimer:public TimerHw{
public:
    using IT = TimerIT;
    using Mode = TimerMode;
    using TrgoSource = TimerTrgoSource;
    using Callback = std::function<void(void)>;
private:
    std::array<Callback, 8> cbs_;
protected:
    TIM_TypeDef * instance;

    uint getClk();
    void enableRcc(const bool en);
    void remap(const uint8_t rm);
    
    Callback & getCallback(const IT it){
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

    __inline void invokeCallback(const IT it){
        auto & cb = getCallback(it);
        EXECUTE(cb);
    }
public:

    BasicTimer(TIM_TypeDef * _base):instance(_base){;}

    void init(const uint32_t ferq, const Mode mode = Mode::Up, const bool en = true);
    void init(const uint16_t period, const uint16_t cycle, const Mode mode = Mode::Up, const bool en = true);
    void enable(const bool en = true);

    void enableIt(const IT it,const NvicPriority request, const bool en = true);
    void enableArrSync(const bool _sync = true){TIM_ARRPreloadConfig(instance, FunctionalState(_sync));}

    auto & inst() {return instance;}

    volatile uint16_t & cnt(){return instance->CNT;}
    volatile uint16_t & arr(){return instance->ATRLR;}

    void attach(const IT it, const NvicPriority & priority, auto && cb, const bool en = true){
        bindCb(it, std::forward<decltype(cb)>(cb));
        enableIt(it, priority, en);
    }

    void attach(const IT it, const NvicPriority & priority, std::nullptr_t cb){
        attach(it, priority, nullptr, false);
    }

    void bindCb(const IT ch, auto && cb){
        getCallback(ch) = std::forward<decltype(cb)>(cb);
    }

    BasicTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM6
    BASIC_TIMER_FRIEND_DECL(6)
    #endif

    #ifdef ENABLE_TIM7
    BASIC_TIMER_FRIEND_DECL(7)
    #endif
};

class GenericTimer:public BasicTimer{
protected:
    TimerOC channels[4];

    void onCCInterrupt();
private:
    void onItInterrupt();
public:
    GenericTimer(TIM_TypeDef * _base):
            BasicTimer(_base),
            channels{
                TimerOC(instance, TimerChannel::ChannelIndex::CH1),
                TimerOC(instance, TimerChannel::ChannelIndex::CH2),
                TimerOC(instance, TimerChannel::ChannelIndex::CH3),
                TimerOC(instance, TimerChannel::ChannelIndex::CH4)
            }{;}

    void initAsEncoder(const Mode mode = Mode::Up);
    void enableSingle(const bool _single = true);
    void setTrgoSource(const TrgoSource source);
    
    TimerOC & oc(const size_t index);

    TimerChannel & operator [](const int index);
    TimerChannel & operator [](const TimerChannel::ChannelIndex channel){return channels[uint8_t(channel) >> 1];}
    [[deprecated]] GenericTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM2
    GENERIC_TIMER_FRIEND_DECL(2)
    #endif

    #ifdef ENABLE_TIM3
    GENERIC_TIMER_FRIEND_DECL(3)
    #endif

    #ifdef ENABLE_TIM4
    GENERIC_TIMER_FRIEND_DECL(4)
    #endif

    #ifdef ENABLE_TIM5
    GENERIC_TIMER_FRIEND_DECL(5)
    #endif


};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t calculateDeadzone(const uint deadzone_ns);

    TimerOCN n_channels[3];

    __fast_inline void onUpdateInterrupt(){invokeCallback(IT::Update);}
    __fast_inline void onBreakInterrupt(){invokeCallback(IT::Break);}
    __fast_inline void onTriggerInterrupt(){invokeCallback(IT::Trigger);}
    __fast_inline void onComInterrupt(){invokeCallback(IT::COM);}

public:
    using LockLevel = TimerBdtrLockLevel;

    AdvancedTimer(TIM_TypeDef * _base):
            GenericTimer(_base),
            n_channels{
                TimerOCN(instance, TimerChannel::ChannelIndex::CH1N),
                TimerOCN(instance, TimerChannel::ChannelIndex::CH2N),
                TimerOCN(instance, TimerChannel::ChannelIndex::CH3N),
            }{;}

    void initBdtr(const uint32_t ns = 200, const LockLevel level = LockLevel::Off);
    void enableCvrSync(const bool _sync = true){TIM_CCPreloadControl(instance, FunctionalState(_sync));}

    void setDeadZone(const uint32_t ns);
    void setRepeatTimes(const uint8_t rep){instance->RPTCR = rep;}

    TimerChannel & operator [](const int index);

    TimerChannel & operator [](const TimerChannel::ChannelIndex ch);
    TimerOCN & ocn(const int index){return n_channels[index - 1];}
    [[deprecated]] AdvancedTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM1
    ADVANCED_TIMER_FRIEND_DECL(1);
    #endif

    #ifdef ENABLE_TIM8
    ADVANCED_TIMER_FRIEND_DECL(8);
    #endif

    #ifdef ENABLE_TIM9
    ADVANCED_TIMER_FRIEND_DECL(9);
    #endif

    #ifdef ENABLE_TIM10
    ADVANCED_TIMER_FRIEND_DECL(10);
    #endif
};


#undef BASIC_TIMER_FRIEND_DECL
#undef GENERIC_TIMER_FRIEND_DECL
#undef ADVANCED_TIMER_FRIEND_DECL

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
