#include "src/testbench/tb.h"

#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/Option.hpp"

#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "src/testbench/tb.h"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"


// https://www.cnblogs.com/wchmcu/p/18781096
// https://www.cnblogs.com/wchmcu/p/18325273

#define UART hal::uart2

template<size_t N>
class InterleavedPwmGen{

};

static constexpr real_t ONE_BY_3 = real_t(1.0 / 3);
static constexpr real_t TWO_BY_3 = real_t(2.0 / 3);
static constexpr real_t FOUR_BY_3 = real_t(4.0 / 3);
static constexpr real_t FIVE_BY_3 = real_t(5.0 / 3);

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr int roundi(const iq_t<P> iq){
    return int(iq + iq_t<Q>(0.5));
}

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
            default: __builtin_unreachable();
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

    // [[nodiscard]]
    // constexpr real_t to_duty() const {
    //     switch(kind_){
    //         default: __builtin_unreachable();
    //         case UpJust:            return 0.0_r;
    //         case UpFirst:           return ONE_BY_3;
    //         case UpSecond:          return TWO_BY_3;
    //         case DownJust:          return 1.0_r;   //INVERSED
    //         case DownFirst:         return TWO_BY_3;//inversed
    //         case DownSecond:        return ONE_BY_3;//inversed
    //     }
    // }

    constexpr size_t to_idx() const {
        return static_cast<size_t>(kind_);
    }
    constexpr auto kind() const {return kind_;}
private:
    Kind kind_;
};

namespace ymd{
    OutputStream & operator <<(OutputStream & os, TrigOccasion::Kind kind){
        switch(kind){
            // default: PANIC(uint8_t(kind));
            default: __builtin_unreachable();
            case TrigOccasion::UpJust:            return os << "UpJust";
            case TrigOccasion::UpFirst:           return os << "UpFirst";
            case TrigOccasion::UpSecond:          return os << "UpSecond";
            case TrigOccasion::DownJust:          return os << "DownJust";
            case TrigOccasion::DownFirst:         return os << "DownFirst";
            case TrigOccasion::DownSecond:        return os << "DownSecond";
        }
    }

    OutputStream & operator <<(OutputStream & os, TrigOccasion occ){
        return os << occ.kind();
    }
}

template<size_t Q>
bool not_in_one(const iq_t<Q> qv){
    // return (qv.value.to_i32() & (~uint32_t((1 << Q) - 1)));
    if(qv < -0.001_r) return true;
    if(qv > 1.001_r) return true;
    return false;
}

template<size_t Q>
bool is_in_one(const iq_t<Q> qv){
    return not is_in_one(qv);
}
    
class InterleavedPwmGen3 final{
// private:
public:
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


        static constexpr Option<CalcResult> calc_in_120_v(const TrigOccasion occasion, const real_t duty){
            switch(occasion.kind()){
                case TrigOccasion::UpFirst:
                    return Some(CalcResult{OcMode::ActiveForever, ONE_BY_3 + duty, OcMode::ActiveBelowCvr, DISEN});
                case TrigOccasion::UpSecond:
                    return Some(CalcResult{OcMode::ActiveBelowCvr, 0, OcMode::ActiveBelowCvr, EN});
                case TrigOccasion::DownSecond:
                    return Some(CalcResult{OcMode::ActiveBelowCvr, ONE_BY_3 - duty, OcMode::ActiveAboveCvr, EN});
                default: return None;
            }
        }

        static constexpr Option<CalcResult> calc_in_240_v(const TrigOccasion occasion, const real_t duty){
            switch(occasion.kind()){
                case TrigOccasion::UpSecond:
                    return Some(CalcResult{OcMode::InactiveForever, duty - ONE_BY_3, OcMode::ActiveBelowCvr, EN});
                case TrigOccasion::DownSecond:
                    return Some(CalcResult{OcMode::ActiveForever, ONE_BY_3 + duty, OcMode::ActiveBelowCvr, EN});
                default: return None;
            }
        }

        static constexpr Option<CalcResult> calc_in_360_v(const TrigOccasion occasion, const real_t duty){
            switch(occasion.kind()){
                case TrigOccasion::UpSecond:
                    return Some(CalcResult{OcMode::ActiveForever, FIVE_BY_3 - duty, OcMode::ActiveAboveCvr, EN});
                case TrigOccasion::DownFirst:
                    return Some(CalcResult{OcMode::InactiveForever, duty - ONE_BY_3, OcMode::ActiveBelowCvr, DISEN});
                case TrigOccasion::DownSecond:
                    return Some(CalcResult{OcMode::ActiveForever, 1, OcMode::ActiveForever, EN});
                default: return None;
            }
        }

        static constexpr Option<CalcResult> calc_v(const TrigOccasion occasion, const real_t duty){
            const auto duty_span = calc_duty_span(duty);

            switch(duty_span){
            case DutySpan::_0: 
                return Some(CalcResult{OcMode::InactiveForever, 0, OcMode::InactiveForever, EN});
            case DutySpan::_120:
                return calc_in_120_v(occasion, duty);
            case DutySpan::_240:
                return calc_in_240_v(occasion, duty);
            case DutySpan::_360:
                return calc_in_360_v(occasion, duty);
                default: return None;
            }
        }

        // static constexpr Option<CalcResult> calc_in_120_w(const TrigOccasion occasion, const real_t duty){
        //     switch(occasion.kind()){
        //         case TrigOccasion::UpJust:
        //             return Some(CalcResult{OcMode::InactiveForever, TWO_BY_3 - duty, OcMode::ActiveAboveCvr});
        //         case TrigOccasion::UpSecond:
        //             return Some(CalcResult{OcMode::ActiveForever, TWO_BY_3 + duty, OcMode::ActiveBelowCvr});
        //         case TrigOccasion::DownJust:
        //             return Some(CalcResult{OcMode::InactiveForever, 0, OcMode::InactiveForever});
        //         default: return None;
        //     }
        // }

        // static constexpr Option<CalcResult> calc_in_240_w(const TrigOccasion occasion, const real_t duty){
        //     switch(occasion.kind()){
        //         case TrigOccasion::UpJust:
        //             return Some(CalcResult{OcMode::InactiveForever, TWO_BY_3 - duty, OcMode::ActiveAboveCvr});
        //         case TrigOccasion::DownJust:
        //             return Some(CalcResult{OcMode::ActiveForever, FOUR_BY_3 - duty, OcMode::ActiveAboveCvr});
        //         default: return None;
        //     }
        // }

        // static constexpr Option<CalcResult> calc_in_360_w(const TrigOccasion occasion, const real_t duty){
        //     switch(occasion.kind()){
        //         case TrigOccasion::UpJust:
        //             return Some(CalcResult{OcMode::ActiveForever, 1, OcMode::ActiveForever});
        //         case TrigOccasion::UpSecond:
        //             return Some(CalcResult{OcMode::ActiveForever, FOUR_BY_3 - duty, OcMode::ActiveAboveCvr});
        //         case TrigOccasion::DownSecond:
        //             return Some(CalcResult{OcMode::InactiveForever, duty - TWO_BY_3, OcMode::ActiveBelowCvr});
        //         default: return None;
        //     }
        // }
        // static constexpr Option<CalcResult> calc_w(const TrigOccasion occasion, const real_t duty){
        //     const auto duty_span = calc_duty_span(duty);

        //     switch(duty_span){
        //         case DutySpan::_0: 
        //             return Some(CalcResult{OcMode::InactiveForever, 0, OcMode::InactiveForever});
        //         case DutySpan::_120:
        //             return calc_in_120_w(occasion, duty);
        //         case DutySpan::_240:
        //             return calc_in_240_w(occasion, duty);
        //         case DutySpan::_360:
        //             return calc_in_360_w(occasion, duty);
        //             default: return None;
        //     }
        // }

        static constexpr DutySpan calc_duty_span(const real_t duty){
            constexpr auto NEAR_ONE = 0.999_r;
            return static_cast<DutySpan>(uint8_t(NEAR_ONE + duty * 3)); 
        }

        void test();
    };

public:
    InterleavedPwmGen3(hal::AdvancedTimer & timer):
        timer_(timer), 
        pwm_u_(timer.oc(1)), 
        pwm_v_(timer.oc(2)), 
        pwm_w_(timer.oc(3)),
        pwm_trig_(timer.oc(4)),
        pwm_un_(timer.ocn(1)), 
        pwm_vn_(timer.ocn(2)), 
        pwm_wn_(timer.ocn(3))
        {
    }

    void on_update_isr(){
        test_gpio.toggle();

        const auto tim_arr = timer_.arr();
        const auto tim_cnt = timer_.cnt();

        const bool is_on_top = tim_cnt > (tim_arr >> 1);
        const auto curr_occasion = [&]() -> TrigOccasion
        {
            if(trig_occasion_opt.is_some()){
                return trig_occasion_opt
                    .unwrap()
                    .iter_next_on_upisr();
            }else{
                return is_on_top ? TrigOccasion::DownJust : TrigOccasion::UpJust; 
            }
        }();

        trig_occasion_opt = Some(curr_occasion);

        on_duty_update();
    }

    void on_ch4_isr(){
        test_gpio.toggle();

        if(trig_occasion_opt.is_some()){
            const auto last_occasion = trig_occasion_opt.unwrap();
            const auto curr_occasion = last_occasion.iter_next_on_ch4isr();

            trig_occasion_opt = Some(curr_occasion);

            const auto next_duty = [curr_occasion]{
                switch(curr_occasion.kind()){
                    case TrigOccasion::UpFirst: return TWO_BY_3;
                    case TrigOccasion::UpSecond: return TWO_BY_3;
                    case TrigOccasion::DownFirst: return ONE_BY_3;
                    case TrigOccasion::DownSecond: return ONE_BY_3;
                    default: PANIC{"ch4isr", curr_occasion.kind()};
                }
            }();

            pwm_trig_.set_duty(next_duty);

            on_sector_update(curr_occasion);
        }
    }

    struct Config{
        uint freq;
        uint deadzone_ns;
    };

    void init(const Config & cfg){

        timer_.init(cfg.freq, TimerCountMode::CenterAlignedDualTrig, false);
        timer_.set_deadzone_ns(cfg.deadzone_ns);
        timer_.enable_cc_ctrl_sync(false);

        pwm_u_.init({
            .oc_mode = TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = EN
        });
    
        pwm_un_.init({});

        pwm_v_.init({
            .oc_mode = TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN
        });

        pwm_vn_.init({});
    
        pwm_w_.init({
            .oc_mode = TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN
        });

        pwm_wn_.init({});
    
        pwm_trig_.init({
            .oc_mode = TimerOcMode::ActiveBelowCvr,
            .cvr_sync_en = DISEN,
            .install_en = EN,
        });

        pwm_trig_.set_duty(TWO_BY_3);
        test_gpio.outpp();
        timer_.enable();
    }

    using Duty = std::array<real_t, 3>;

    __no_inline void set_duty(const Duty duty){
        // duty_cmd_ = duty;
        // std::copy(duty_cmd_.data(), duty.data(), duty.size());

        duty_cmd_shadow_[0] = duty[0];
        duty_cmd_shadow_[1] = duty[1];
        duty_cmd_shadow_[2] = duty[2];

        // DEBUG_PRINTLN(duty_cmd_);
    }

    void set_freq(const uint freq){
        timer_.set_freq(freq);
    }

    void set_deadzone_ns(const uint ns){
        timer_.set_deadzone_ns(ns);
    }

    void static_test();
private:
    hal::AdvancedTimer & timer_;
    hal::TimerOC & pwm_u_;
    hal::TimerOC & pwm_v_;
    hal::TimerOC & pwm_w_;
    hal::TimerOC & pwm_trig_;

    hal::TimerOCN & pwm_un_;
    hal::TimerOCN & pwm_vn_;
    hal::TimerOCN & pwm_wn_;

    hal::Gpio & test_gpio = portA[12];

    Option<TrigOccasion> trig_occasion_opt = None;
    Duty duty_cmd_shadow_ = {0.0_r, 0.0_r, 0.0_r};
    Duty duty_cmd_ = {0.0_r, 0.0_r, 0.0_r};

    void on_duty_update(){
        
        duty_cmd_[0] = duty_cmd_shadow_[0];
        duty_cmd_[1] = duty_cmd_shadow_[1];
        duty_cmd_[2] = duty_cmd_shadow_[2];
    
        test_gpio.toggle();
    }
    void on_sector_update(const TrigOccasion curr_ocs){



        pwm_u_.set_duty(duty_cmd_[0]);
        set_pwm_shift_120(pwm_v_, curr_ocs, duty_cmd_[1]);
        set_pwm_shift_240(pwm_w_, curr_ocs, duty_cmd_[2]);



        test_gpio.toggle();

    }

    void set_pwm_shift_60(TimerOC & pwm, const TrigOccasion curr_ocs, const real_t duty){
        const auto duty_span = SlaveAlgoHelper::calc_duty_span(duty);

        switch(duty_span){
        case DutySpan::_120:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(ONE_BY_3 + duty);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    break;
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(EN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_duty(1_r);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(ONE_BY_3 - duty);
                    break;
                default: break;
            };break;
        case DutySpan::_240:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(duty - ONE_BY_3);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(ONE_BY_3 + duty);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    break;
                default: break;
            }; break;
        case DutySpan::_360:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_duty(0);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(FIVE_BY_3 - duty);
                    break;
                case TrigOccasion::DownFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(duty - ONE_BY_3);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    break;
                default: break;
            }; break;
            default: break;
        }
    }

    void set_pwm_shift_120(TimerOC & pwm, const TrigOccasion curr_ocs, const real_t duty){
        const auto duty_span = SlaveAlgoHelper::calc_duty_span(duty);

        switch(duty_span){
        case DutySpan::_120:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(TWO_BY_3 + duty);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(0);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(1);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(TWO_BY_3 - duty);
                    break;
                default: break;
            };break;
        case DutySpan::_240:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(0);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(FOUR_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(1);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(TWO_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                default: break;
            }; break;
        case DutySpan::_360:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(0);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(FOUR_BY_3 - duty);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(duty - TWO_BY_3);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(1);
                    break;
                default: break;
            }; break;
            default: break;
        }
    }

    void set_pwm_shift_240(TimerOC & pwm, const TrigOccasion curr_ocs, const real_t duty){
        const auto duty_span = SlaveAlgoHelper::calc_duty_span(duty);

        switch(duty_span){
        case DutySpan::_120:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(0);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(TWO_BY_3 + duty);
                    break;
                case TrigOccasion::DownFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(TWO_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(1);
                    break;
                default: break;
            };break;
        case DutySpan::_240:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(0);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(TWO_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(1);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(FOUR_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    break;
                default: break;
            }; break;
        case DutySpan::_360:
            switch(curr_ocs.kind()){
                case TrigOccasion::UpFirst:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::InactiveForever);
                    pwm.set_duty(FOUR_BY_3 - duty);
                    pwm.set_oc_mode(OcMode::ActiveAboveCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(0);
                    break;
                case TrigOccasion::DownSecond:
                    pwm.enable_cvr_sync(DISEN);
                    pwm.set_oc_mode(OcMode::ActiveForever);
                    pwm.set_duty(1);
                    pwm.set_oc_mode(OcMode::ActiveBelowCvr);
                    pwm.enable_cvr_sync(EN);
                    pwm.set_duty(duty - TWO_BY_3);
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

void tb1_pwm_always_high(hal::AdvancedTimer & timer){
    // timer.init(2_KHz, TimerCountMode::CenterAlignedDualTrig, false);

    InterleavedPwmGen3 pwm_gen{timer};
    pwm_gen.init({
        .freq = 20_KHz,
        // .freq = 10_KHz,
        // .freq = 5_KHz,
        .deadzone_ns = 200
    });

    timer.attach(TimerIT::Update, {0,0}, [&]{
        pwm_gen.on_update_isr();
    });

    timer.attach(TimerIT::CC4, {0,0}, [&]{
        pwm_gen.on_ch4_isr();
    });



    while(true){
        const auto t = time();

        [[maybe_unused]]
        const auto [st, ct] = sincospu(700 * t);

        // const auto mt = st * 0.4_r ;
        // const auto mt = 0.2917_r + ONE_BY_3;
        // const auto mt = 0.2917_r;
        // const auto mt = 0.68_r;
        // const auto mt = 0.8_r;

        // DEBUG_PRINTLN(InterleavedPwmGen3::SlaveAlgoHelper::calc_in_360_w(TrigOccasion::DownJust, mt).unwrap());
        // const auto mt = 0.4_r;
        // const auto mt = 0.24_r;
        static constexpr const real_t depth = 0.4_r;
        const auto [u, v, w] = drivers::SVM(st * depth, ct * depth);
        pwm_gen.set_duty({u, v, w});
        // DEBUG_PRINTLN(
        //     real_t(timer.oc(1)),
        //     real_t(timer.oc(2)),
        //     real_t(timer.oc(3))
        // );

        DEBUG_PRINTLN_IDLE(
            u, 
            v, 
            w,
            real_t(timer.oc(1))
        );
        // delay(1);
        udelay(100);
        // pwm_gen.set_duty({0.2_r, 0.4_r, 0.6_r});
        // pwm_gen.set_duty({0.6_r, 0.8_r, 0.9_r});

        // DEBUG_PRINTLN(t, real_t(pwm_trig_));
        // if(trig_occasion_opt.is_some())
        //     DEBUG_PRINTLN(trig_occasion_opt.unwrap().kind());
        // delay(1);
    }
}

void phase3pwm_main(void)
{
    
    UART.init(DEBUG_UART_BAUD);
    DEBUGGER.retarget(&UART);
    // tb1();
    // TIM1_Phase_shift_Init2();
    tb1_pwm_always_high(hal::timer1);
}
