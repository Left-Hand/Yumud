#pragma once

#include "hal/timer/timer_channel.hpp"
#include "hal/timer/timer_utils.hpp"


namespace ymd::hal{
class Gpio;

class TimerOut: public TimerChannel{
protected:
    TimerOut(TIM_TypeDef * inst, const ChannelNth nth):
        TimerChannel(inst, nth){;}
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
    TimerOC(TIM_TypeDef * inst, const ChannelNth nth):
        TimerOut(inst, nth), 
            cvr_(from_channel_to_cvr(inst, nth)), 
            arr_(inst_->ATRLR){;}

    void init(const TimerOcPwmConfig & config);

    void set_oc_mode(const Mode mode);
    void enable_cvr_sync(const Enable en = EN);
    
    Gpio & io();

    __fast_inline volatile uint16_t & cvr() {return cvr_;}
    __fast_inline volatile uint16_t & arr() {return arr_;}

    __fast_inline volatile uint16_t cvr() const {return cvr_;}
    __fast_inline volatile uint16_t arr() const {return arr_;}

    __fast_inline void set_dutycycle(const real_t duty){cvr_ = int(duty * arr_);}
    __fast_inline void set_cvr(const uint cvr){cvr_ = cvr;}
    __fast_inline real_t get_dutycycle(){return iq_t<8>(cvr_) / int(arr_);}


};

class TimerOCN final:public TimerOut{
public:
    TimerOCN(
        TIM_TypeDef * _base, 
        const ChannelNth nth):
        TimerOut(_base, nth)
        {;}

    void init(const TimerOcnPwmConfig & cfg);

    Gpio & io();
};


//将输出通道和互补输出通道作为一个双极性pwm物理层的两个通道
//当输入为正时 原始输出通道作业 互补输出通道关闭
//当输入为负时 原始输出通道关闭 互补输出通道作业
class TimerOcMirror final:public PwmIntf{
public:
    TimerOcMirror(
        TimerOC & oc, 
        TimerOCN & ocn):
        oc_(oc), ocn_(ocn){;}
    void set_dutycycle(const real_t value){
        const bool polar = value > 0;
        if(last_polar_ != polar){
            last_polar_ = polar;

            if(polar){
                oc_.enable_output(EN);
                ocn_.enable_output(DISEN);
            }else{
                oc_.enable_output(DISEN);
                ocn_.enable_output(EN);
            }
        }
        oc_.set_dutycycle(abs(value));
    }
private:
    TimerOC & oc_;
    TimerOCN & ocn_;
    bool last_polar_ = false;
};


//将两个输出通道作为一个双极性pwm物理层的两个通道
//当输入为正时 高端输出通道作业 低端输出通道关闭
//当输入为负时 高端输出通道关闭 低端输出通道作业
class TimerOcPair final:public PwmIntf{
public:
    TimerOcPair(TimerOC & pos_oc, TimerOC & neg_oc):
        pos_oc_(pos_oc),
        neg_oc_(neg_oc){;}

    __fast_inline void set_dutycycle(const real_t value){
        const bool is_negative = signbit(value);
        const auto zero_value = real_t(is_inversed_);
        const auto abs_value = is_inversed_ ? (1 - ABS(value)) : ABS(value);

        if(is_negative){
            pos_oc_.set_dutycycle(zero_value);
            neg_oc_.set_dutycycle(abs_value);
        }else{
            pos_oc_.set_dutycycle(abs_value);
            neg_oc_.set_dutycycle(zero_value);
        }
    }

    void inverse(const Enable en){
        is_inversed_ = en == EN;
    }

    auto & pos_channel() {
        return pos_oc_;
    }

    auto & neg_channel(){
        return neg_oc_;
    }
private:
    TimerOC & pos_oc_;
    TimerOC & neg_oc_;
    bool is_inversed_ = false;
};


};