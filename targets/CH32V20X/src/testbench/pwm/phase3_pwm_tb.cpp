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





// https://www.cnblogs.com/wchmcu/p/18781096
// https://www.cnblogs.com/wchmcu/p/18325273

#define UART hal::uart2

template<size_t N>
class InterleavedPwmGen{

};

template<>
class InterleavedPwmGen<3>{

};

static constexpr real_t ONE_BY_3 = real_t(1.0 / 3);
static constexpr real_t TWO_BY_3 = real_t(2.0 / 3);

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

    // [[nodiscard]]
    // // constexpr 
    // TrigOccasion iter_update_isr() const {
    //     switch(kind_){
    //         case UpJust: return UpFirst;
    //         case UpSecond: return DownJust;
    //         case DownJust: return DownFirst;
    //         case DownSecond: return UpJust;
    //         default: PANIC{kind_};
    //         // default: PANIC();
    //     }
    // }

    // [[nodiscard]]
    // // constexpr 
    // TrigOccasion iter_by_arr_and_cnt_ch4_isr(
    //     const uint16_t arr, const uint16_t cnt
    // ) const {
    //     const real_t duty_x3 = iq_t<8>(cnt * 3) / arr;
    //     const uint8_t level = uint8_t(roundi(duty_x3));

    //     switch(level){
    //         default: PANIC(level, kind_, cnt, arr);
    //         case 1: {
    //             switch(kind_){
    //                 case UpJust: return UpSecond;
    //                 case UpFirst: return UpSecond;
    //                 case DownJust: return DownSecond;
    //                 case DownFirst: return DownSecond; 
    //                 default: PANIC(level, kind_);
    //             }
    //         };
    //         case 2: {
    //             switch(kind_){
    //                 case UpFirst: return DownJust;
    //                 case UpSecond: return DownJust;
    //                 case DownJust: return DownSecond;
    //                 case DownFirst: return DownSecond; 
    //                 default: PANIC(level, kind_);
    //             }
    //         };
    //     }
    // }

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

void tb1_pwm_always_high(hal::AdvancedTimer & timer){
    // timer.init(2_KHz, TimerCountMode::CenterAlignedDualTrig, false);
    timer.init(20_KHz, TimerCountMode::CenterAlignedDualTrig, false);
    timer.enable_cc_ctrl_sync(false);
    auto & pwm_u = timer.oc(1);
    auto & pwm_v = timer.oc(2);
    auto & pwm_w = timer.oc(3);
    auto & pwm_trig = timer.oc(4);

    pwm_u.init({
        .oc_mode = TimerOcMode::ActiveBelowCvr,
        .cvr_sync_en = EN
    });

    pwm_v.init({
        // .oc_mode = TimerOcMode::ActiveForever,
        .oc_mode = TimerOcMode::ActiveBelowCvr,
        .cvr_sync_en = DISEN
    });

    pwm_w.init({
        .oc_mode = TimerOcMode::InactiveForever,
        .cvr_sync_en = DISEN
    });

    pwm_trig.init({
        .oc_mode = TimerOcMode::ActiveBelowCvr,
        .cvr_sync_en = DISEN,
        .install_en = DISEN
        // .install_en = EN
    });

    // pwm_trig.enable_cvr_sync(DISEN);
    pwm_trig.set_duty(0.66_r);

    auto & trig_gpio = portA[12];
    trig_gpio.outpp();

    Option<TrigOccasion> trig_occasion_opt = None;
    // Option<TrigOccasion> trig_occasion_opt = Some(TrigOccasion::DownJust);
    // Option<TrigOccasion> trig_occasion_opt = Some(TrigOccasion::DownJust);

    timer.enable();

    #if 1
    timer.attach(TimerIT::Update, {0,0}, [&]{
        const auto tim_arr = timer.arr();
        const auto tim_cnt = timer.cnt();

        bool is_on_top = tim_cnt > (tim_arr >> 1);

        if(trig_occasion_opt.is_some()){
            const auto last_occasion = trig_occasion_opt.unwrap();
            const auto curr_occasion = last_occasion.iter_next_on_upisr();
            trig_occasion_opt = Some(curr_occasion);
        }else{
            if(is_on_top){
                trig_occasion_opt = Some(TrigOccasion(
                    TrigOccasion::DownJust));
            }else{
                trig_occasion_opt = Some(TrigOccasion(
                    TrigOccasion::UpJust));
            }
        }

        trig_gpio.toggle();

        if(is_on_top){
            pwm_v.set_duty(0.5_r);
            pwm_v.set_oc_mode(TimerOcMode::ActiveBelowCvr);
        }else{
            pwm_v.set_oc_mode(TimerOcMode::InactiveForever);
        }
    });


    timer.attach(TimerIT::CC4, {0,0}, [&]{
        const auto tim_arr = timer.arr();
        const auto tim_cnt = timer.cnt();

        bool is_on_top = tim_cnt > (tim_arr >> 1);

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
            pwm_trig.set_duty(next_duty);
        }

        trig_gpio.toggle();
        if(is_on_top){
        //     pwm_u.set_duty(0.5_r);
        //     pwm_u.set_oc_mode(TimerOcMode::ActiveBelowCvr);
        // }else{
        //     pwm_u.set_oc_mode(TimerOcMode::InactiveForever);

        }
    });
    #endif

    
    // timer.attach(TimerIT::CC4, {0,0}, [&]{
    //     const auto t = time();
    //     static bool is_high = false;
    //     trig_gpio.toggle();
    //     if(is_high){
    //         pwm_trig.set_duty(0.33333_r);
    //         pwm_v.set_duty(0.33333_r);
    //         DEBUG_PRINTLN(t, real_t(pwm_trig), pwm_trig.cvr(), timer.arr());
    //         is_high = false;
    //     }else{
    //         pwm_trig.set_duty(0.6666_r);
    //         pwm_v.set_duty(0.63333_r);
    //         DEBUG_PRINTLN(t, real_t(pwm_trig), pwm_trig.cvr(), timer.arr());
    //         is_high = true;
    //     }
    // });


    while(true){
        const auto t = time();

        [[maybe_unused]]
        const auto [st, ct] = sincospu(10 * t);

        const auto mt = st * 0.4_r + 0.5_r;

        pwm_u.set_duty(0.1_r);
        // pwm_u.set_duty(mt);
        pwm_v.set_duty(mt);
        pwm_w.set_duty(mt);


        // DEBUG_PRINTLN(t, real_t(pwm_trig));
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
