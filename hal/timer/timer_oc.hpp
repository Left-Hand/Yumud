#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"


namespace ymd::hal{
class Gpio;

class [[nodiscard]] TimerOutBase: public TimerChannel{
protected:
    TimerOutBase(TIM_TypeDef * inst, const ChannelSelection nth):
        TimerChannel(inst, nth){;}
    void plant_to_pin(const Enable en);
public:
    void set_valid_level(const BoolLevel level);
    void enable_output(const Enable en);


};

struct [[nodiscard]] TimerOcPwmConfig final{
    TimerOcMode oc_mode = TimerOcMode::ActiveBelowCvr;
    Enable cvr_sync_en = EN;
    BoolLevel valid_level = HIGH;
    Enable out_en = EN;
    Enable plant_en = EN;
};

struct [[nodiscard]] TimerOcnPwmConfig final{
    Enable out_en = EN;
    Enable plant_en = EN;
};

class [[nodiscard]] TimerOC final:public PwmIntf, public TimerOutBase{
public:
    using Mode = TimerOcMode;
protected:
    volatile uint16_t & cvr_;
    volatile uint16_t & arr_;
public:
    TimerOC(TIM_TypeDef * inst, const ChannelSelection nth):
        TimerOutBase(inst, nth), 
            cvr_(from_channel_to_cvr(inst, nth)), 
            arr_(inst_->ATRLR){;}

    void init(const TimerOcPwmConfig & config);

    void set_oc_mode(const Mode mode);
    void enable_cvr_sync(const Enable en);
    
    Gpio io();

    __fast_inline volatile uint16_t & cvr() {return cvr_;}
    __fast_inline volatile uint16_t & arr() {return arr_;}

    __fast_inline volatile uint16_t cvr() const {return cvr_;}
    __fast_inline volatile uint16_t arr() const {return arr_;}

    __fast_inline void set_dutycycle(const real_t duty){cvr_ = int(duty * arr_);}
    __fast_inline void set_cvr(const uint cvr){cvr_ = cvr;}
    __fast_inline fixed_t<16, int32_t> get_dutycycle(){return fixed_t<16, int32_t>(cvr_) / uint32_t(arr_);}


};

class [[nodiscard]] TimerOCN final:public TimerOutBase{
public:
    TimerOCN(
        TIM_TypeDef * _base, 
        const ChannelSelection nth):
        TimerOutBase(_base, nth)
        {;}

    void init(const TimerOcnPwmConfig & cfg);

    Gpio io();
};


};