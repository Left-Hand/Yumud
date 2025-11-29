#pragma once

#include "timer_oc.hpp"

namespace ymd::hal{


//将输出通道和互补输出通道(共轭通道)作为一个双极性pwm物理层的两个通道
//当输入为正时 原始输出通道作业 互补输出通道关闭
//当输入为负时 原始输出通道关闭 互补输出通道作业
class [[nodiscard]] BipolarityTimerOcConjugate final:public PwmIntf{
public:
    BipolarityTimerOcConjugate(
        TimerOC & oc, 
        TimerOCN & ocn
    ):
        oc_(oc), ocn_(ocn){;}
    void set_dutycycle(const iq16 value){
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
class [[nodiscard]] BipolarityTimerOcPair final:public PwmIntf{
public:
    BipolarityTimerOcPair(TimerOC & pos_oc, TimerOC & neg_oc):
        pos_oc_(pos_oc),
        neg_oc_(neg_oc){;}

    __fast_inline void set_dutycycle(const iq16 value){
        const bool is_negative = signbit(value);
        const iq16 zero_value = is_inversed_ ? 1 : 0;
        const auto abs_value = (is_inversed_) ? (1 - ABS(value)) : ABS(value);

        if(is_negative){
            pos_oc_.set_dutycycle(zero_value);
            neg_oc_.set_dutycycle(abs_value );
        }else{
            pos_oc_.set_dutycycle(abs_value );
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


}