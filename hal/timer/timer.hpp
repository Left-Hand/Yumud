#pragma once

#include "timer_oc.hpp"
#include "timer_utils.hpp"

#ifdef HDW_SXX32

#define ADVANCED_TIMER_IT(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void);\
__interrupt void TIM##x##_UP_IRQHandler(void);\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void);\
__interrupt void TIM##x##_CC_IRQHandler(void);\
}\


#define GENERIC_TIMER_IT(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#define BASIC_TIMER_IT(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\



#define ADVANCED_TIMER_FRIEND(x)\
friend void ::TIM##x##_BRK_IRQHandler(void);\
friend void ::TIM##x##_UP_IRQHandler(void);\
friend void ::TIM##x##_TRG_COM_IRQHandler(void);\
friend void ::TIM##x##_CC_IRQHandler(void);\


#define GENERIC_TIMER_FRIEND(x)\
friend void ::TIM##x##_IRQHandler(void);\


#define BASIC_TIMER_FRIEND(x)\
friend void ::TIM##x##_IRQHandler(void);\


#ifdef ENABLE_TIM1
ADVANCED_TIMER_IT(1)
#endif

#ifdef ENABLE_TIM2
GENERIC_TIMER_IT(2)
#endif

#ifdef ENABLE_TIM3
GENERIC_TIMER_IT(3)
#endif

#ifdef ENABLE_TIM4
GENERIC_TIMER_IT(4)
#endif

#ifdef ENABLE_TIM5
GENERIC_TIMER_IT(5)
#endif

#ifdef ENABLE_TIM6
BASIC_TIMER_IT(6)
#endif

#ifdef ENABLE_TIM7
BASIC_TIMER_IT(7)
#endif

#ifdef ENABLE_TIM8
ADVANCED_TIMER_IT(8)
#endif

#ifdef ENABLE_TIM9
ADVANCED_TIMER_IT(9)
#endif

#ifdef ENABLE_TIM10
ADVANCED_TIMER_IT(10)
#endif

#undef ADVANCED_TIMER_IT
#undef GENERIC_TIMER_IT
#undef BASIC_TIMER_IT


namespace ymd{
class TimerHw{};

class BasicTimer:public TimerHw{
public:
    using IT = TimerUtils::IT;
    using Mode = TimerUtils::Mode;
    using TrgoSource = TimerUtils::TrgoSource;

protected:
    TIM_TypeDef * instance;

    using Callback = std::function<void(void)>;

    std::array<Callback, 8> cbs;

    uint getClk();
    void enableRcc(const bool en);

    void onUpdateHandler();
    void onBreakHandler();
    void onTriggerComHandler();
    void handleIt(const IT it);
public:

    
    BasicTimer(TIM_TypeDef * _base):instance(_base){;}

    void init(const uint32_t ferq, const Mode mode = Mode::Up, const bool en = true);
    void init(const uint16_t period, const uint16_t cycle, const Mode mode = Mode::Up, const bool en = true);
    void enable(const bool en = true);

    void enableIt(const IT it,const NvicPriority request, const bool en = true);
    void enableArrSync(const bool _sync = true){TIM_ARRPreloadConfig(instance, (FunctionalState)_sync);}

    auto & inst() {return instance;}

    volatile uint16_t & cnt(){return instance->CNT;}
    volatile uint16_t & arr(){return instance->ATRLR;}

    virtual void bindCb(const IT ch, std::function<void(void)> && cb){
        cbs[CTZ((uint8_t)ch)] = std::move(cb);\
    }

    BasicTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM6
    BASIC_TIMER_FRIEND(6)
    #endif

    #ifdef ENABLE_TIM7
    BASIC_TIMER_FRIEND(7)
    #endif
};

class GenericTimer:public BasicTimer{
protected:
    TimerOC channels[4];

    void onCCHandler();
private:
    void onItHandler();
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
    
    TimerChannel & ch(const size_t index);
    
    TimerOC & oc(const size_t index);

    virtual TimerChannel & operator [](const int index){return ch(index);}
    virtual TimerChannel & operator [](const TimerChannel::ChannelIndex channel){return channels[(uint8_t)channel >> 1];}

    GenericTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM2
    GENERIC_TIMER_FRIEND(2)
    #endif

    #ifdef ENABLE_TIM3
    GENERIC_TIMER_FRIEND(3)
    #endif

    #ifdef ENABLE_TIM4
    GENERIC_TIMER_FRIEND(4)
    #endif

    #ifdef ENABLE_TIM5
    GENERIC_TIMER_FRIEND(5)
    #endif


};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t calculateDeadzone(const uint deadzone_ns);

    TimerOCN n_channels[3];
public:
    using LockLevel = TimerUtils::BdtrLockLevel;

    AdvancedTimer(TIM_TypeDef * _base):
            GenericTimer(_base),
            n_channels{
                TimerOCN(instance, TimerChannel::ChannelIndex::CH1N),
                TimerOCN(instance, TimerChannel::ChannelIndex::CH2N),
                TimerOCN(instance, TimerChannel::ChannelIndex::CH3N),
            }{;}

    void initBdtr(const uint32_t ns = 200, const LockLevel level = LockLevel::Off);
    void enableCvrSync(const bool _sync = true){TIM_CCPreloadControl(instance, (FunctionalState)_sync);}
    void setDeadZone(const uint32_t ns);
    void setRepeatTimes(const uint8_t rep){instance->RPTCR = rep;}

    TimerChannel & operator [](const int index) override;

    TimerChannel & operator [](const TimerChannel::ChannelIndex ch) override;
    TimerOCN & ocn(const int index){return n_channels[CLAMP(index, 1, 3) - 1];}
    AdvancedTimer & operator = (const real_t duty){instance->CNT = uint16_t(instance->ATRLR * duty); return *this;}

    #ifdef ENABLE_TIM1
    ADVANCED_TIMER_FRIEND(1);
    #endif

    #ifdef ENABLE_TIM8
    ADVANCED_TIMER_FRIEND(8);
    #endif

    #ifdef ENABLE_TIM9
    ADVANCED_TIMER_FRIEND(9);
    #endif

    #ifdef ENABLE_TIM10
    ADVANCED_TIMER_FRIEND(10);
    #endif
};
}


#ifdef ENABLE_TIM1
inline ymd::AdvancedTimer timer1{TIM1};
#endif

#ifdef ENABLE_TIM2
inline ymd::GenericTimer timer2{TIM2};
#endif

#ifdef ENABLE_TIM3
inline ymd::GenericTimer timer3{TIM3};
#endif

#ifdef ENABLE_TIM4
inline ymd::GenericTimer timer4{TIM4};
#endif

#ifdef ENABLE_TIM5
inline ymd::GenericTimer timer5{TIM5};
#endif

#ifdef ENABLE_TIM6
inline ymd::BasicTimer timer6{TIM6};
#endif

#ifdef ENABLE_TIM7
inline ymd::BasicTimer timer7{TIM7};
#endif

#ifdef ENABLE_TIM8
inline ymd::AdvancedTimer timer8{TIM8};
#endif

#ifdef ENABLE_TIM9
inline ymd::AdvancedTimer timer9{TIM9};
#endif

#ifdef ENABLE_TIM10
inline ymd::AdvancedTimer timer10{TIM10};
#endif

#endif
