#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"


namespace ymd::hal{
class Gpio;

class [[nodiscard]] TimerOutBase: public TimerChannel{
protected:
    TimerOutBase(void * inst, const ChannelSelection sel):
        TimerChannel(inst, sel){;}
public:
    void set_valid_level(const BoolLevel level);
    void enable_output(const Enable en);


};

struct [[nodiscard]] TimerOcPwmConfig final{
    TimerOcMode oc_mode;
    Enable cvr_sync_en;
    BoolLevel valid_level;
    Enable out_en;

    using Self = TimerOcPwmConfig;
    static constexpr Self from_default() {
        return Self{
            .oc_mode = TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = EN,
            .valid_level = HIGH,
            .out_en = EN,
            // .plant_en = EN,
        };
    }
};

struct [[nodiscard]] TimerOcnPwmConfig final{
    using Self = TimerOcnPwmConfig;
    Enable out_en;

    static constexpr Self from_default() {
        return Self{
            .out_en = EN,
        };
    }
};

class [[nodiscard]] TimerOC final:public PwmIntf, public TimerOutBase{
public:
    using Mode = TimerOcMode;
    using Config = TimerOcPwmConfig;
public:
    TimerOC(void * inst, const ChannelSelection sel);
    void init(const Config & config);

    static constexpr Config default_config(){
        return Config::from_default();
    }

    void set_oc_mode(const Mode mode);
    void enable_cvr_sync(const Enable en);
    
    Gpio io();

    [[nodiscard]] __fast_inline volatile uint16_t & cvr() {return cvr_;}
    [[nodiscard]] __fast_inline volatile uint16_t & arr() {return arr_;}

    [[nodiscard]] __fast_inline volatile uint16_t cvr() const {return cvr_;}
    [[nodiscard]] __fast_inline volatile uint16_t arr() const {return arr_;}

    __fast_inline void set_dutycycle(const uq16 dutycycle){
        cvr_ = static_cast<uint16_t>(dutycycle * arr_);
    }
    __fast_inline void set_cvr(const uint cvr){cvr_ = cvr;}
    __fast_inline uq16 get_dutycycle(){return uq16(cvr_) / uint32_t(arr_);}
protected:
    volatile uint16_t & cvr_;
    volatile uint16_t & arr_;

};

class [[nodiscard]] TimerOCN final:public TimerOutBase{
public:
    using Config = TimerOcnPwmConfig;
    TimerOCN(
        void * inst, 
        const ChannelSelection sel):
        TimerOutBase(inst, sel)
        {;}

    void init(const Config & cfg);
    static constexpr Config default_config(){
        return Config::from_default();
    }

    Gpio io();
};


};