#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/gpio/gpio.hpp"

class TimerOC:public TimerChannel, public PwmChannelAccessible{
public:
    enum class Mode:uint16_t{
        Timing = TIM_OCMode_Timing,
        Active = TIM_OCMode_Active,
        Inactive = TIM_OCMode_Inactive,
        Toggle = TIM_OCMode_Toggle,
        UpValid = TIM_OCMode_PWM1,
        DownValid = TIM_OCMode_PWM2
    };
protected:
    volatile uint16_t & m_cvr;
    volatile uint16_t & m_arr;
public:
    TimerOC(TIM_TypeDef * _instance, const Channel _channel):TimerChannel(_instance, _channel), m_cvr(from_channel_to_cvr(_channel)), m_arr(instance->ATRLR){;}

    void init() override{init(true, Mode::UpValid);}
    void init(const bool & install = true, const Mode & mode = Mode::UpValid);

    void enable(const bool & en = true);

    void setMode(const Mode & _mode);
    void enableSync(const bool & _sync = true);
    void setPolarity(const bool & pol);
    void setIdleState(const bool & state);
    void installToPin(const bool & en = true);

    __fast_inline TimerOC & operator = (const real_t & duty){
        if(duty == 0) {m_cvr = 0;}
        else if(duty == 1) {m_cvr = m_arr - 1;}
        else {m_cvr = int(duty * m_arr);}
        return *this;}

    __fast_inline volatile uint16_t & cnt() override {return instance->CNT;}
    __fast_inline volatile uint16_t & cvr() override {return m_cvr;}
    __fast_inline volatile uint16_t & arr() override {return m_arr;}
    __fast_inline operator real_t(){return real_t(m_cvr) / real_t(m_arr);}
};

class TimerOCN:public TimerChannel{
public:
    TimerOCN(TIM_TypeDef * _base, const Channel _channel):TimerChannel(_base, _channel){;}
};
