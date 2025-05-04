#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"


namespace ymd::hal{
class Gpio;

class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * _instance, const ChannelIndex _channel):TimerChannel(_instance, _channel){;}
    void install_to_pin(const Enable en = EN);
public:
    void set_valid_level(const BoolLevel level);
    void enable_output(const Enable en = EN);

    virtual Gpio & io() = 0;
};

struct TimerOcPwmConfig final{
    TimerOcMode oc_mode = TimerOcMode::ActiveBelowCvr;
    Enable cvr_sync_en = EN;
    BoolLevel valid_level = HIGH;
    Enable out_en = EN;
    Enable install_en = EN;
};

struct TimerOcnPwmConfig final{
    Enable out_en = EN;
    Enable install_en = EN;
};

class TimerOC final:public PwmIntf, public TimerOut{
public:
    using Mode = TimerOcMode;
protected:
    volatile uint16_t & cvr_;
    volatile uint16_t & arr_;
public:
    TimerOC(TIM_TypeDef * _instance, const ChannelIndex _channel):
        TimerOut(_instance, _channel), 
            cvr_(from_channel_to_cvr(_instance, _channel)), 
            arr_(instance->ATRLR){;}

    void init(const TimerOcPwmConfig & config);

    void set_oc_mode(const Mode mode);
    void enable_cvr_sync(const Enable en = EN);
    
    Gpio & io();
    __fast_inline volatile uint16_t & cvr() {return cvr_;}
    __fast_inline volatile uint16_t & arr() {return arr_;}

    __fast_inline volatile uint16_t cvr() const {return cvr_;}
    __fast_inline volatile uint16_t arr() const {return arr_;}

    __fast_inline void set_duty(const real_t duty){cvr_ = int(duty * arr_);}
    __fast_inline void set_cvr(const uint cvr){cvr_ = cvr;}
    __fast_inline TimerOC & operator = (const real_t duty) 
        override {set_duty(duty); return *this;}
    __fast_inline operator real_t(){return iq_t<8>(cvr_) / int(arr_);}
};

class TimerOCN final:public TimerOut{
public:
    TimerOCN(TIM_TypeDef * _base, const ChannelIndex _channel):TimerOut(_base, _channel){;}
    void init(const TimerOcnPwmConfig & cfg);

    Gpio & io();
};

class TimerOcMirror final:public PwmIntf{
protected:
    TimerOC & oc_;
    TimerOCN & ocn_;
    bool last_polar = false;
public:
    TimerOcMirror(TimerOC & oc, TimerOCN & ocn):
        oc_(oc), ocn_(ocn){;}

    TimerOcMirror & operator = (const real_t value) override{
        set_duty(value);
        return *this;
    }

    void set_duty(const real_t value){
        const bool polar = value > 0;
        if(last_polar != polar){
            last_polar = polar;

            if(polar){
                oc_.enable_output(EN);
                ocn_.enable_output(DISEN);
            }else{
                oc_.enable_output(DISEN);
                ocn_.enable_output(EN);
            }
        }
        oc_ = abs(value);
    }
};

class TimerOcPair final:public PwmIntf{
protected:
    TimerOC & oc_;
    TimerOC & ocn_;
    bool inversed = false;
public:
    TimerOcPair(TimerOC & oc, TimerOC & ocn):
        oc_(oc),
        ocn_(ocn){;}
    TimerOcPair & operator = (const real_t value) override{
        const bool is_minus = signbit(value);
        const auto abs_value = inversed ? (1 - ABS(value)) : ABS(value);
        const auto zero_value = real_t(inversed);

        if(is_minus){
            oc_ = zero_value;
            ocn_ = abs_value;
        }else{
            oc_ = abs_value;
            ocn_ = zero_value;
        }
        
        return *this;
    }

    void inverse(const bool en){
        inversed = en;
    }
};


};