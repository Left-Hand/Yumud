#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/gpio/gpio.hpp"

class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * _instance, const Channel _channel):TimerChannel(_instance, _channel){;}
public:
    void installToPin(const bool en = true);
    void enableSync(const bool _sync = true);
    void setPolarity(const bool pol);
    void setOutputState(const bool s);
    void setIdleState(const bool state);
    void enable(const bool en = true);
};

class TimerOC:public TimerOut, public PwmChannel{
public:
    using Mode = TimerUtils::OcMode;
protected:
    volatile uint16_t & cvr_;
    volatile uint16_t & arr_;
public:
    TimerOC(TIM_TypeDef * _instance, const Channel _channel):TimerOut(_instance, _channel), cvr_(from_channel_to_cvr(_channel)), arr_(instance->ATRLR){;}

    void init() override{init(Mode::UpValid, true);}
    void init(const Mode mode, const bool install = true);
    void setMode(const Mode _mode);

    auto & io(){return TimerUtils::getPin(instance, channel);}
    __fast_inline volatile uint16_t & cvr(){return cvr_;}
    __fast_inline volatile uint16_t & arr(){return arr_;}

    __fast_inline TimerOC & operator = (const real_t duty) override{cvr_ = int(duty * arr_);return *this;}
    __fast_inline operator real_t(){return real_t(cvr_) / real_t(arr_);}
};

class TimerOCN:public TimerOut{
protected:
    // volatile uint16_t cvr_ = 0;
    // volatile uint16_t arr_ = 0;
public:
    TimerOCN(TIM_TypeDef * _base, const Channel _channel):TimerOut(_base, _channel){;}
    void init() {installToPin();}
};
