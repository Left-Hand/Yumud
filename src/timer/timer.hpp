#ifndef __TIMER_HPP__

#define __TIMER_HPP__

#include "src/platform.h"
#include "src/nvic/nvic.hpp"
#include "timer_oc.hpp"
#include <functional>

class Timer{
public:
    enum class TimerMode:uint16_t{
        Up = TIM_CounterMode_Up,
        Down = TIM_CounterMode_Down,
        CenterAlignedDownTrig = TIM_CounterMode_CenterAligned1,
        CenterAlignedCenterUpTrig = TIM_CounterMode_CenterAligned2,
        CenterAlignedCenterDualTrig = TIM_CounterMode_CenterAligned3
    };

    enum class IT:uint8_t{
        Update = TIM_IT_Update,
        CC1 = TIM_IT_CC1,
        CC2 = TIM_IT_CC2,
        CC3 = TIM_IT_CC3,
        CC4 = TIM_IT_CC4,
        COM = TIM_IT_COM,
        Trigger = TIM_IT_Trigger,
        Break = TIM_IT_Break,
    };

    virtual Timer & operator = (const uint16_t _val) = 0;
};

class BasicTimer:public Timer{
protected:
    TIM_TypeDef * instance;

    IRQn ItToIrq(const IT & it){
        switch((uint32_t)instance){
            #ifdef HAVE_TIM1
            case TIM1_BASE:
                switch(it){
                    case IT::Update:
                        return TIM1_UP_IRQn;
                    case IT::CC1:
                    case IT::CC2:
                    case IT::CC3:
                    case IT::CC4:
                        return TIM1_CC_IRQn;
                    case IT::Trigger:
                    case IT::COM:
                        return TIM1_TRG_COM_IRQn;
                    case IT::Break:
                        return TIM1_BRK_IRQn;
                }
                break;
            #endif

            #ifdef HAVE_TIM2
            case TIM2_BASE:
                return TIM2_IRQn;
            #endif

            #ifdef HAVE_TIM3
            case TIM3_BASE:
                return TIM3_IRQn;
            #endif

            #ifdef HAVE_TIM4
            case TIM4_BASE:
                return TIM4_IRQn;
            #endif

            default:
                break;
        }
        return Software_IRQn;
    }
public:
    BasicTimer(TIM_TypeDef * _base):instance(_base){;}

    void init(const uint32_t & ferq, const TimerMode & mode = TimerMode::Up, const bool & en = true);
    void init(const uint16_t & period, const uint16_t & cycle, const TimerMode & mode = TimerMode::Up, const bool & en = true);
    void enable(const bool & en = true);

    // void enableIt(const IT & it,const uint8_t & pre = 0, const uint8_t & sub = 0, const bool & en = true){
    //     NvicRequest request(ItToIrq(it), pre, sub);
    //     request.enable(en);
    //     TIM_ITConfig(instance, (uint16_t)it, (FunctionalState)en);
    // }

    void enableIt(const IT it,const NvicPriority & request, const bool en = true){
        NvicPriority::enable(request, ItToIrq(it), en);
        TIM_ITConfig(instance, (uint16_t)it, (FunctionalState)en);
    }

    void enableSync(const bool _sync = true){TIM_ARRPreloadConfig(instance, (FunctionalState)_sync);}

    virtual void bindCb(const IT & ch, const std::function<void(void)> & cb) = 0;
    BasicTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
    operator uint16_t() const {return instance->CNT;}
};

class GenericTimer:public BasicTimer{
protected:
    TimerOutChannelPosOnChip channels[4];
public:
    GenericTimer(TIM_TypeDef * _base):BasicTimer(_base),
            channels{
                TimerOutChannelPosOnChip(instance, TimerChannelOnChip::Channel::CH1),
                TimerOutChannelPosOnChip(instance, TimerChannelOnChip::Channel::CH2),
                TimerOutChannelPosOnChip(instance, TimerChannelOnChip::Channel::CH3),
                TimerOutChannelPosOnChip(instance, TimerChannelOnChip::Channel::CH4)
            }{;}
    void initAsEncoder(const TimerMode mode = TimerMode::Up);
    void enableSingle(const bool _single = true);
    TimerOutChannelPosOnChip & ch(const int & index){return channels[CLAMP(index, 1, 4) - 1];}
    // TimerOutChannelOnChip getChannel(const TimerOutChannelOnChip::Channel ch){return TimerOutChannelOnChip(instance, ch);}
    virtual TimerOutChannelPosOnChip & operator [](const int & index){return ch(index);}
    virtual TimerOutChannelPosOnChip & operator [](const TimerOutChannelOnChip::Channel & channel){return channels[(uint8_t)channel >> 1];}
    GenericTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t caculate_dead_zone(uint32_t ns);

    TimerOutChannelNegOnChip n_channels[3];
public:

    enum class LockLevel:uint16_t{
        Off = TIM_LOCKLevel_OFF,
        Low = TIM_LOCKLevel_1,
        Medium = TIM_LOCKLevel_2,
        High = TIM_LOCKLevel_3

    };
    // Bdtr getBdtr();
    void initBdtr(const uint32_t & ns = 200, const LockLevel & level = LockLevel::Off);
    void setDeadZone(const uint32_t & ns);


    // TimerOutChannelOnChip & operator [](const int index) override{
    //     bool is_co = index < 0;
    //     if(is_co) return co_channels[CLAMP(-index, 1, 3) - 1];
    //     else return channels[CLAMP(index, 1, 4) - 1];
    // }

    // TimerOutChannelOnChip & operator [](const TimerChannelOnChip::Channel ch) override {
    //     bool is_co = (uint8_t) ch & 0b1;
    //     if(is_co){
    //         return co_channels[((uint8_t)ch - 1) >> 1];
    //     }else{
    //         return channels[(uint8_t)ch >> 1];
    //     }
    // }

    TimerOutChannelNegOnChip & chn(const int & index){return n_channels[CLAMP(index, 1, 3) - 1];}
    AdvancedTimer(TIM_TypeDef * _base):GenericTimer(_base),
            n_channels{
                TimerOutChannelNegOnChip(instance, TimerChannelOnChip::Channel::CH1N),
                TimerOutChannelNegOnChip(instance, TimerChannelOnChip::Channel::CH2N),
                TimerOutChannelNegOnChip(instance, TimerChannelOnChip::Channel::CH3N),
            }{;}

    AdvancedTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
};

#endif