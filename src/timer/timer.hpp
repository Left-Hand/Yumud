#ifndef __TIMER_HPP__

#define __TIMER_HPP__

#include "src/platform.h"
#include "timer_oc.hpp"

class Timer{
public:
    enum class TimerMode:uint16_t{
        Up = TIM_CounterMode_Up,
        Down = TIM_CounterMode_Down,
        CenterAlignedDownTrig = TIM_CounterMode_CenterAligned1,
        CenterAlignedCenterUpTrig = TIM_CounterMode_CenterAligned2,
        CenterAlignedCenterDualTrig = TIM_CounterMode_CenterAligned3
    };

    virtual Timer & operator = (const uint16_t _val) = 0;
};

class BasicTimer:public Timer{
protected:
    TIM_TypeDef * base;
public:
    BasicTimer(TIM_TypeDef * _base):base(_base){;}
    void init(const uint16_t arr, const uint16_t psc = 0, const TimerMode mode = TimerMode::Up);
    void enable(const bool en = true);

    void configIT(const uint16_t _it,const bool en = true){TIM_ITConfig(base, _it, (FunctionalState)en);}
    void configSync(const bool _sync){TIM_ARRPreloadConfig(base, (FunctionalState)_sync);}
    void configSingle(const bool _single){TIM_SelectOnePulseMode(base, _single ? TIM_OPMode_Repetitive:TIM_OPMode_Single);}

    BasicTimer & operator = (const uint16_t _val) override {base->CNT = _val;return *this;}
};

class GenericTimer:public BasicTimer{
    TimerOC getChannel(const TimerOC::Channel ch){return TimerOC(base, ch);}
    GenericTimer & operator = (const uint16_t _val) override {base->CNT = _val;return *this;}
};

class AdvancedTimer:public GenericTimer{
    GenericTimer & operator = (const uint16_t _val) override {base->CNT = _val;return *this;}
};

#endif