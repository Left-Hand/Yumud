#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"


namespace ymd::hal{
class Gpio;

class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * _instance, const ChannelIndex _channel):TimerChannel(_instance, _channel){;}
    void install_to_pin(const bool en = true);
public:


    TimerOut & set_sync(const bool _sync = true);
    TimerOut & set_polarity(const bool pol);
    TimerOut & set_output_state(const bool s);
    TimerOut & set_idle_state(const bool state);
    TimerOut & enable(const bool en = true);

    virtual Gpio & io() = 0;
};

class TimerOC:public TimerOut, public PwmIntf{
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
    TimerOC & set_mode(const Mode _mode);

    Gpio & io();
    __fast_inline volatile uint16_t & cvr() {return cvr_;}
    __fast_inline volatile uint16_t & arr() {return arr_;}

    __fast_inline volatile uint16_t cvr() const {return cvr_;}
    __fast_inline volatile uint16_t arr() const {return arr_;}

    __fast_inline TimerOC & operator = (const real_t duty) override{cvr_ = int(duty * arr_);return *this;}
    __fast_inline operator real_t(){return real_t(cvr_) / int(arr_);}
};

class TimerOCN:public TimerOut{
public:
    TimerOCN(TIM_TypeDef * _base, const ChannelIndex _channel):TimerOut(_base, _channel){;}
    void init() {install_to_pin();}

    Gpio & io();
};

};