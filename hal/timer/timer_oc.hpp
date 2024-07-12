#pragma once

#include "../hal/timer/timer_channel.hpp"
#include "../hal/timer/timer_utils.hpp"
#include "../hal/gpio/gpio.hpp"

class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * _instance, const Channel _channel):TimerChannel(_instance, _channel){;}
public:
    void installToPin(const bool en = true);
    void enableSync(const bool _sync = true);
    void setPolarity(const bool pol);
    void setIdleState(const bool state);
    void enable(const bool en = true);
};

class TimerOC:public TimerOut,public PwmChannel, public Countable{
public:
    using Mode = TimerUtils::OcMode;
protected:
    volatile uint16_t & m_cvr;
    volatile uint16_t & m_arr;
public:
    TimerOC(TIM_TypeDef * _instance, const Channel _channel):TimerOut(_instance, _channel), m_cvr(from_channel_to_cvr(_channel)), m_arr(instance->ATRLR){;}

    void init() override{init(Mode::UpValid);}
    void init(const Mode mode, const bool install = true);
    void setMode(const Mode _mode);

    auto & io(){return TimerUtils::getPin(instance, channel);}

    __fast_inline TimerOC & operator = (const real_t duty) override{m_cvr = int(duty * m_arr);return *this;}
    __fast_inline operator real_t(){return real_t(m_cvr) / real_t(m_arr);}
    __fast_inline volatile uint16_t & cnt() override {return instance->CNT;}

    __fast_inline volatile uint16_t cnt() const {return instance->CNT;}
    __fast_inline volatile uint16_t & cvr() override {return m_cvr;}
    __fast_inline volatile uint16_t & arr() override {return m_arr;}
};

class TimerOCN:public TimerOut{
protected:
    volatile uint16_t m_cvr = 0;
    volatile uint16_t m_arr = 0;
public:
    TimerOCN(TIM_TypeDef * _base, const Channel _channel):TimerOut(_base, _channel){;}
    void init() {installToPin();}
};
