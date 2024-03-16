#ifndef __TIMER_HPP__

#define __TIMER_HPP__

#include "src/platform.h"
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
public:
    BasicTimer(TIM_TypeDef * _base):instance(_base){;}

    void init(const uint32_t ferq, const TimerMode mode = TimerMode::Up);
    void init(const uint16_t period, const uint16_t cycle, const TimerMode mode = TimerMode::Up);
    void enable(const bool en = true);

    void enableIT(const uint16_t _it,const bool en = true){TIM_ITConfig(instance, _it, (FunctionalState)en);}
    void enableSync(const bool _sync = true){TIM_ARRPreloadConfig(instance, (FunctionalState)_sync);}

    virtual void bindCb(const IT & ch, const std::function<void(void)> & cb) = 0;
    BasicTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
    operator uint16_t() const {return instance->CNT;}
};

class GenericTimer:public BasicTimer{
public:
    GenericTimer(TIM_TypeDef * _base):BasicTimer(_base){;}
    void initAsEncoder(const TimerMode mode = TimerMode::Up);
    void enableSingle(const bool _single = true);
    TimerOC getChannel(const TimerOC::Channel ch){return TimerOC(instance, ch);}
    GenericTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t caculate_dead_zone(uint32_t ns);
public:

    enum class LockLevel:uint16_t{
        Off = TIM_LOCKLevel_OFF,
        Low = TIM_LOCKLevel_1,
        Medium = TIM_LOCKLevel_2,
        High = TIM_LOCKLevel_3

    };
    // Bdtr getBdtr();
    void initBdtr(const LockLevel level = LockLevel::Off, const uint32_t ns = 200);
    void setDeadZone(const uint32_t ns);
    AdvancedTimer(TIM_TypeDef * _base):GenericTimer(_base){;}
    AdvancedTimer & operator = (const uint16_t _val) override {instance->CNT = _val;return *this;}
};

#endif