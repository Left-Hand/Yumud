#pragma once

#include "core/math/realmath.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/Option.hpp"

#include "hal/timer/timer_oc.hpp"
#include "hal/timer/timer.hpp"



namespace ymd::digipw{

class InterleavedPwmGen3 final{
public:

    static constexpr real_t ONE_BY_3 = real_t(1.0 / 3);
    static constexpr real_t TWO_BY_3 = real_t(2.0 / 3);
    static constexpr real_t FOUR_BY_3 = real_t(4.0 / 3);
    static constexpr real_t FIVE_BY_3 = real_t(5.0 / 3);

    class TrigOccasion{
    public:
        enum class Kind:uint8_t{
            UpJust, 
            UpFirst, 
            UpSecond, 
            DownJust,
            DownFirst,
            DownSecond
        };
    
        using enum Kind;
    
        constexpr TrigOccasion(Kind kind):kind_(kind){}
    
        constexpr bool operator == (const Kind kind) const {
            return kind_ == kind;}
        constexpr bool operator == (const TrigOccasion other) const {
            return kind_ == other.kind_;}
    
        // constexpr bool 
        [[nodiscard]]
        constexpr TrigOccasion iter_next() const {
            switch(kind_){
                case UpJust: return UpFirst;
                case UpFirst: return UpSecond;
                case UpSecond: return DownJust;
                case DownJust: return DownFirst;
                case DownFirst: return DownSecond;
                case DownSecond: return UpJust;
                default: PANIC{kind_};
                // default: PANIC();
            }
        }
    
        [[nodiscard]]
        constexpr TrigOccasion iter_next_on_upisr() const {
            switch(kind_){
                case UpSecond: return DownJust;
                case DownSecond: return UpJust;
                // default: __builtin_unreachable();
                default: while(true);
            }
        }
    
        [[nodiscard]]
        constexpr TrigOccasion iter_next_on_ch4isr() const {
            switch(kind_){
                case UpJust: return UpFirst;
                case UpFirst: return UpSecond;
                case DownJust: return DownFirst;
                case DownFirst: return DownSecond;
                default: __builtin_unreachable();
            }
        }
    
    
        [[nodiscard]]
        constexpr bool is_aligned() const {
            return (kind_ == TrigOccasion::UpJust) or 
                (kind_ == TrigOccasion::DownJust);
        }
    
        [[nodiscard]]
        constexpr bool is_first() const {
            return (kind_ == TrigOccasion::UpFirst) or 
                (kind_ == TrigOccasion::DownFirst);
        }
    
        [[nodiscard]]
        constexpr bool is_second() const {
            return (kind_ == TrigOccasion::UpSecond) or 
                (kind_ == TrigOccasion::DownSecond);
        }
    
        constexpr size_t to_idx() const {
            return static_cast<size_t>(kind_);
        }
        constexpr auto kind() const {return kind_;}
    private:
        Kind kind_;
    };

    enum class DutySpan:uint8_t{
        _0,
        _120,
        _240,
        _360
    };

    using OcMode = hal::TimerOcMode;

    struct CalcResult{
        OcMode oc_mode_temp;
        real_t oc_duty;
        OcMode oc_mode2;
        Enable sync_en;
        friend OutputStream & operator <<(OutputStream & os, const CalcResult & self){
            return os << self.oc_duty;
        }
    };

    struct SlaveAlgoHelper{

        static constexpr DutySpan calc_duty_span(const real_t duty){
            constexpr auto NEAR_ONE = 0.999_r;
            return static_cast<DutySpan>(uint8_t(NEAR_ONE + duty * 3)); 
        }

        void test();
    };

public:
    InterleavedPwmGen3(hal::AdvancedTimer & timer):
        timer_(timer), 
        pwm_u_(timer.oc<1>()), 
        pwm_v_(timer.oc<2>()), 
        pwm_w_(timer.oc<3>()),
        pwm_trig_(timer.oc<4>()),
        pwm_un_(timer.ocn<1>()), 
        pwm_vn_(timer.ocn<2>()), 
        pwm_wn_(timer.ocn<3>())
        {
    }


    struct Config{
        uint32_t freq;
        Nanoseconds deadzone_ns;
    };

    void init(const Config & cfg){

        timer_.init({
            cfg.freq, 
            hal::TimerCountMode::CenterAlignedDualTrig, 
            DISEN}
        );
        timer_.set_deadzone_ns(cfg.deadzone_ns);
        timer_.enable_cc_ctrl_sync(DISEN);

        pwm_u_.init({
            .oc_mode = hal::TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = EN
        });
    
        pwm_un_.init({});

        pwm_v_.init({
            .oc_mode = hal::TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN
        });

        pwm_vn_.init({});
    
        pwm_w_.init({
            .oc_mode = hal::TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN
        });

        pwm_wn_.init({});
    
        pwm_trig_.init({
            .oc_mode = hal::TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN,
            .install_en = EN,
        });

        pwm_trig_.set_duty(TWO_BY_3);
        test_gpio.outpp();
        timer_.enable();
    }

    using Duty = std::array<real_t, 3>;

    __no_inline void set_duty(const Duty duty){
        duty_cmd_shadow_[0] = duty[0];
        duty_cmd_shadow_[1] = duty[1];
        duty_cmd_shadow_[2] = duty[2];
    }

    void set_freq(const uint32_t freq){
        timer_.set_freq(freq);
    }

    void set_deadzone_ns(const Nanoseconds ns){
        timer_.set_deadzone_ns(ns);
    }

    void static_test();

    void on_update_isr(){
        test_gpio.clr();

        const auto tim_arr = timer_.arr();
        const auto tim_cnt = timer_.cnt();

        const bool is_on_top = tim_cnt > (tim_arr >> 1);
        const auto curr_occasion = [&]() -> TrigOccasion {
            if(trig_occasion_opt_.is_some()){
                return trig_occasion_opt_
                    .unwrap()
                    .iter_next_on_upisr();
            }else{
                return is_on_top ? TrigOccasion::DownJust : TrigOccasion::UpJust; 
            }
        }();

        trig_occasion_opt_ = Some(curr_occasion);

        if(is_on_top){
            duty_cmd_[0] = duty_cmd_shadow_[0];
            duty_cmd_[1] = duty_cmd_shadow_[1];
            duty_cmd_[2] = duty_cmd_shadow_[2];
        }

        test_gpio.set();
    }

    void on_ch4_isr(){
        test_gpio.clr();

        if(trig_occasion_opt_.is_none()) return;

        const auto last_occasion = trig_occasion_opt_.unwrap();
        const auto curr_occasion = last_occasion.iter_next_on_ch4isr();

        trig_occasion_opt_ = Some(curr_occasion);

        const auto next_duty = [curr_occasion]{
            switch(curr_occasion.kind()){
                // default: PANIC{"ch4isr", curr_occasion.kind()};
                default: __builtin_unreachable();
                case TrigOccasion::UpFirst: return TWO_BY_3;
                case TrigOccasion::UpSecond: return TWO_BY_3;
                case TrigOccasion::DownFirst: return ONE_BY_3;
                case TrigOccasion::DownSecond: return ONE_BY_3;
            }
        }();

        pwm_trig_.set_duty(next_duty);

        const auto tim_arr = timer_.arr();

        pwm_u_.set_duty(duty_cmd_[0]);
        set_pwm_shift_120(pwm_v_, duty_cmd_[1], curr_occasion, tim_arr);
        set_pwm_shift_240(pwm_w_, duty_cmd_[2], curr_occasion, tim_arr);

        test_gpio.set();
    }

private:
    hal::AdvancedTimer & timer_;
    hal::TimerOC & pwm_u_;
    hal::TimerOC & pwm_v_;
    hal::TimerOC & pwm_w_;
    hal::TimerOC & pwm_trig_;

    hal::TimerOCN & pwm_un_;
    hal::TimerOCN & pwm_vn_;
    hal::TimerOCN & pwm_wn_;

    hal::Gpio & test_gpio = hal::portA[12];

    Option<TrigOccasion> trig_occasion_opt_ = None;
    Duty duty_cmd_shadow_ = {0.0_r, 0.0_r, 0.0_r};
    Duty duty_cmd_ = {0.0_r, 0.0_r, 0.0_r};

    static void set_pwm_shift_120(
        hal::TimerOC & pwm, 
        const real_t duty,
        const TrigOccasion curr_ocs, 
        const uint32_t arr
    ){
        const auto duty_span = SlaveAlgoHelper::calc_duty_span(duty);

        switch(duty_span){
        case DutySpan::_120:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 + duty)));
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 - duty)));
                    break;
                default: break;
            };break;
        case DutySpan::_240:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (FOUR_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                default: break;
            }; break;
        case DutySpan::_360:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (FOUR_BY_3 - duty)));
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (duty - TWO_BY_3)));
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    break;
                default: break;
            }; break;
            default: break;
        }
    }

    static void set_pwm_shift_240(
        hal::TimerOC & pwm, 
        const real_t duty,
        const TrigOccasion curr_ocs, 
        const uint32_t arr
    ){
        const auto duty_span = SlaveAlgoHelper::calc_duty_span(duty);

        switch(duty_span){
        case DutySpan::_120:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 + duty)));
                    break;
                case TrigOccasion::DownFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    break;
                default: break;
            };break;
        case DutySpan::_240:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (TWO_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (FOUR_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                default: break;
            }; break;
        case DutySpan::_360:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_cvr(uint32_t(arr * (FOUR_BY_3 - duty)));
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (0)));
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_cvr(uint32_t(arr * (1)));
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_cvr(uint32_t(arr * (duty - TWO_BY_3)));
                    break;
                default: break;
            }; break;
            default: break;
        }
    }

};

void InterleavedPwmGen3::static_test(){
    #if 1
    static_assert(SlaveAlgoHelper::calc_duty_span(0.0_r) == DutySpan::_0);
    static_assert(SlaveAlgoHelper::calc_duty_span(0.2_r) == DutySpan::_120);
    static_assert(SlaveAlgoHelper::calc_duty_span(0.6_r) == DutySpan::_240);
    static_assert(SlaveAlgoHelper::calc_duty_span(0.8_r) == DutySpan::_360);
    #endif
}

OutputStream & operator <<(OutputStream & os, InterleavedPwmGen3::TrigOccasion::Kind kind){
    switch(kind){
        default: __builtin_unreachable();
        case InterleavedPwmGen3::TrigOccasion::UpJust:            return os << "UpJust";
        case InterleavedPwmGen3::TrigOccasion::UpFirst:           return os << "UpFirst";
        case InterleavedPwmGen3::TrigOccasion::UpSecond:          return os << "UpSecond";
        case InterleavedPwmGen3::TrigOccasion::DownJust:          return os << "DownJust";
        case InterleavedPwmGen3::TrigOccasion::DownFirst:         return os << "DownFirst";
        case InterleavedPwmGen3::TrigOccasion::DownSecond:        return os << "DownSecond";
    }
}

OutputStream & operator <<(OutputStream & os, InterleavedPwmGen3::TrigOccasion occ){
    return os << occ.kind();
}

}