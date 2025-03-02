#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{
class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * _instance, const ChannelIndex _channel):TimerChannel(_instance, _channel){;}
    void installToPin(const bool en = true);
public:


    TimerOut & sync(const bool _sync = true);
    TimerOut & setPolarity(const bool pol);
    TimerOut & setOutputState(const bool s);
    TimerOut & setIdleState(const bool state);
    TimerOut & enable(const bool en = true);

    virtual Gpio & io() = 0;
};

class TimerOC:public TimerOut, public PwmChannelIntf{
public:
    using Mode = TimerOcMode;
protected:
    volatile uint16_t & cvr_;
    volatile uint16_t & arr_;
public:
    TimerOC(TIM_TypeDef * _instance, const ChannelIndex _channel):
        TimerOut(_instance, _channel), 
            cvr_(from_channel_to_cvr(_instance, _channel)), arr_(instance->ATRLR){;}

    TimerOC & init(const Mode mode = Mode::UpValid, const bool install = true);
    TimerOC & setMode(const Mode _mode);

    Gpio & io();
    __fast_inline volatile uint16_t & cvr() {return cvr_;}
    __fast_inline volatile uint16_t & arr() {return arr_;}

    __fast_inline TimerOC & operator = (const real_t duty) override{cvr_ = int(duty * arr_);return *this;}
    __fast_inline operator real_t(){return real_t(cvr_) / int(arr_);}
};

class TimerOCN:public TimerOut{
public:
    TimerOCN(TIM_TypeDef * _base, const ChannelIndex _channel):TimerOut(_base, _channel){;}
    void init() {installToPin();}

    Gpio & io();
};

};