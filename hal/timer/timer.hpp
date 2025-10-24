#pragma once

#include "core/clock/clock.hpp"
#include "timer_oc.hpp"
#include "timer_utils.hpp"
#include "hal/nvic/nvic.hpp"
#include "core/utils/sumtype.hpp"

#ifdef HDW_SXX32

#define DEF_ADVANCED_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_BRK_IRQHandler(void);\
__interrupt void TIM##x##_UP_IRQHandler(void);\
__interrupt void TIM##x##_TRG_COM_IRQHandler(void);\
__interrupt void TIM##x##_CC_IRQHandler(void);\
}\


#define DEF_GENERIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#define DEF_BASIC_TIMER_IT_FORWARD_DECL(x)\
extern "C"{\
__interrupt void TIM##x##_IRQHandler(void);\
}\


#ifdef ENABLE_TIM1
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(1)
#endif

#ifdef ENABLE_TIM2
DEF_GENERIC_TIMER_IT_FORWARD_DECL(2)
#endif

#ifdef ENABLE_TIM3
DEF_GENERIC_TIMER_IT_FORWARD_DECL(3)
#endif

#ifdef ENABLE_TIM4
DEF_GENERIC_TIMER_IT_FORWARD_DECL(4)
#endif

#ifdef ENABLE_TIM5
DEF_GENERIC_TIMER_IT_FORWARD_DECL(5)
#endif

#ifdef ENABLE_TIM6
DEF_BASIC_TIMER_IT_FORWARD_DECL(6)
#endif

#ifdef ENABLE_TIM7
DEF_BASIC_TIMER_IT_FORWARD_DECL(7)
#endif

#ifdef ENABLE_TIM8
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(8)
#endif

#ifdef ENABLE_TIM9
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(9)
#endif

#ifdef ENABLE_TIM10
DEF_ADVANCED_TIMER_IT_FORWARD_DECL(10)
#endif

#undef DEF_ADVANCED_TIMER_IT_FORWARD_DECL
#undef DEF_GENERIC_TIMER_IT_FORWARD_DECL
#undef DEF_BASIC_TIMER_IT_FORWARD_DECL



#define DEF_ADVANCED_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_BRK_IRQHandler(void);\
friend void ::TIM##x##_UP_IRQHandler(void);\
friend void ::TIM##x##_TRG_COM_IRQHandler(void);\
friend void ::TIM##x##_CC_IRQHandler(void);\


#define DEF_GENERIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


#define DEF_BASIC_TIMER_FRIEND_DECL(x)\
friend void ::TIM##x##_IRQHandler(void);\


namespace ymd::hal{

namespace details{
//pure function, easy test
static constexpr std::tuple<uint16_t, uint16_t> calc_best_arr_and_psc(
	const uint32_t periph_freq, 
	const uint32_t count_freq, 
	std::pair<uint16_t, uint16_t> arr_range
){
    const auto [min_arr, max_arr] = arr_range;
    const uint32_t target_div = periph_freq / count_freq;
    
    auto calc_psc_from_arr = [target_div](const uint16_t arr) -> uint16_t {
        return CLAMP(int(target_div) / (int(arr) + 1) - 1, 0, 0xFFFF);
    };

    [[maybe_unused]]
    auto calc_arr_from_psc = [target_div](const uint16_t psc) -> uint16_t {
        return CLAMP(int(target_div) / (int(psc) + 1) - 1, 0, 0xFFFF);
    };
    
    auto calc_freq_from_arr_and_psc = [periph_freq](const uint16_t arr, const uint16_t psc) -> uint32_t {
        return periph_freq / (arr + 1) / (psc + 1);
    };
    
    const auto min_psc = calc_psc_from_arr(max_arr);
    const auto max_psc = calc_psc_from_arr(min_arr);

    if (min_arr > max_arr) ymd::sys::abort();
    
    struct Best{
        uint16_t arr;
        uint16_t psc;
        uint32_t freq_err;
    };
    
    Best best{max_arr, min_psc, UINT32_MAX};
    for(int arr = max_arr; arr >= min_arr; arr--){
        const auto expect_psc = calc_psc_from_arr(arr);
        if((expect_psc >= max_psc) or (expect_psc < min_psc)) continue;
        
        std::optional<uint32_t> last_freq_;

        // const int psc_start = MAX(min_psc, expect_psc - 5);
        // const int psc_stop = MIN(max_psc, expect_psc + 5);
        // if(psc_start >= psc_stop) continue;

        // for(int psc = psc_start; psc < psc_stop; psc++){
        for(int psc = expect_psc - 2; psc < expect_psc + 2; psc++){
            const auto freq = calc_freq_from_arr_and_psc(arr, psc);
            if(last_freq_.has_value()){
                if((last_freq_.value() - count_freq) * (freq - count_freq) < 0) break;
            }else{
                last_freq_ = freq;
            }
            const auto freq_err = uint32_t(ABS(int(freq) - int(count_freq)));
            if(freq_err < best.freq_err){
                if(freq_err == 0) return {uint16_t(arr), psc};
                best = {uint16_t(arr), uint16_t(psc), freq_err};
            }
        }
    }
    
    if(best.freq_err == UINT32_MAX) ymd::sys::abort();
    return {best.arr, best.psc};
}
}

struct [[nodiscard]] ArrAndPsc{
    using Self = ArrAndPsc;

    uint16_t arr;
    uint16_t psc;

    static constexpr ArrAndPsc from_nearest_count_freq(
        const uint32_t periph_freq,
        const uint32_t count_freq,
        std::pair<uint16_t, uint16_t> arr_range
    ){
        ArrAndPsc ret;
        std::tie(ret.arr, ret.psc) = details::calc_best_arr_and_psc(periph_freq, count_freq, arr_range);
        return ret;
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << self.arr << os.splitter() << self.psc;
    }
};

struct [[nodiscard]] NearestFreq{
    using Self = NearestFreq;
    uint32_t count;

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << self.count;
    }
};

struct [[nodiscard]] TimerCountFreq:
    public Sumtype<ArrAndPsc, NearestFreq>{
};

using TimerEvent = TimerIT;
class BasicTimer{
public:
    using IT = TimerIT;
    using CountMode = TimerCountMode;
    using TrgoSource = TimerTrgoSource;
    using Callback = std::function<void(TimerEvent)>;
private:
    Callback callback_ = nullptr;
protected:
    TIM_TypeDef * inst_;

    uint32_t get_bus_freq();
    void enable_rcc(const Enable en);


    template<IT I>
    __fast_inline void invoke_callback(){
        EXECUTE(callback_, I);
    }

public:
    explicit BasicTimer(TIM_TypeDef * _base):inst_(_base){;}

    struct Config{
        TimerCountFreq count_freq;
        const CountMode count_mode;
    };

    void init(const Config & cfg, const Enable en);
    void set_remap(const uint8_t rm);
    void deinit();

    void enable(const Enable en);
    void set_count_mode(const TimerCountMode mode);

    void set_psc(const uint16_t psc);
    void set_arr(const uint16_t arr);

    void set_freq(const TimerCountFreq freq);

    template<IT I>
    void enable_interrupt(const Enable en){
        TIM_ITConfig(inst_, std::bit_cast<uint8_t>(I), en == EN);
    }

    template<IT I>
    void register_nvic(const NvicPriority request, const Enable en){
        request.with_irqn(details::it_to_irq(inst_, I)).enable(en);
    }

    void enable_arr_sync(const Enable en);
    void enable_psc_sync(const Enable en);
    void enable_cc_ctrl_sync(const Enable en);
    auto & inst() {return inst_;}

    volatile uint16_t & cnt(){return inst_->CNT;}
    volatile uint16_t & arr(){return inst_->ATRLR;}

    template<typename Fn>
    void set_event_callback(Fn && cb){
        callback_ = std::forward<Fn>(cb);
    }

    #ifdef ENABLE_TIM6
    DEF_BASIC_TIMER_FRIEND_DECL(6)
    #endif

    #ifdef ENABLE_TIM7
    DEF_BASIC_TIMER_FRIEND_DECL(7)
    #endif
};

class GenericTimer:public BasicTimer{
protected:
    TimerOC channels[4];

    void on_cc_interrupt();
private:
    void on_it_interrupt();
public:
    explicit GenericTimer(TIM_TypeDef * _base):
        BasicTimer(_base),
        channels{
            TimerOC(inst_, TimerChannel::ChannelSelection::CH1),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH2),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH3),
            TimerOC(inst_, TimerChannel::ChannelSelection::CH4)
        }{;}

    void init_as_encoder(const CountMode mode = CountMode::Up);
    void enable_single(const Enable en);
    void set_trgo_source(const TrgoSource source);

    template<size_t I>
    requires(I >= 1 and I <= 4)
    volatile uint16_t & cvr(){
        switch(I){
            case 1: return inst_->CH1CVR;
            case 2: return inst_->CH2CVR;
            case 3: return inst_->CH3CVR;
            case 4: return inst_->CH4CVR;
            default: __builtin_unreachable();
        }
    }

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOC & oc(){
        return channels[I - 1];
    }

    #ifdef ENABLE_TIM2
    DEF_GENERIC_TIMER_FRIEND_DECL(2)
    #endif

    #ifdef ENABLE_TIM3
    DEF_GENERIC_TIMER_FRIEND_DECL(3)
    #endif

    #ifdef ENABLE_TIM4
    DEF_GENERIC_TIMER_FRIEND_DECL(4)
    #endif

    #ifdef ENABLE_TIM5
    DEF_GENERIC_TIMER_FRIEND_DECL(5)
    #endif


};

class AdvancedTimer:public GenericTimer{
protected:
    uint8_t calculate_deadzone(const Nanoseconds deadzone_ns);

    TimerOCN n_channels[3];

public:
    using LockLevel = TimerBdtrLockLevel;

    explicit AdvancedTimer(TIM_TypeDef * _base):
            GenericTimer(_base),
            n_channels{
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH1N),
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH2N),
                TimerOCN(inst_, TimerChannel::ChannelSelection::CH3N),
            }{;}

    void init_bdtr(const Nanoseconds ns, const LockLevel level = LockLevel::Off);

    void set_deadzone_ns(const Nanoseconds ns);
    void set_repeat_times(const uint8_t rep){inst_->RPTCR = rep;}

    template<size_t I>
    requires(I >= 1 and I <= 4)
    TimerOCN & ocn(){return n_channels[I - 1];}

    #ifdef ENABLE_TIM1
    DEF_ADVANCED_TIMER_FRIEND_DECL(1);
    #endif

    #ifdef ENABLE_TIM8
    DEF_ADVANCED_TIMER_FRIEND_DECL(8);
    #endif

    #ifdef ENABLE_TIM9
    DEF_ADVANCED_TIMER_FRIEND_DECL(9);
    #endif

    #ifdef ENABLE_TIM10
    DEF_ADVANCED_TIMER_FRIEND_DECL(10);
    #endif


};


#undef DEF_BASIC_TIMER_FRIEND_DECL
#undef DEF_GENERIC_TIMER_FRIEND_DECL
#undef DEF_ADVANCED_TIMER_FRIEND_DECL


}

#endif
