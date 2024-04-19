#ifndef __TIMER_OC_HPP__
#define __TIMER_OC_HPP__

#include "timer_channel.hpp"
#include "src/gpio/gpio.hpp"
#include "real.hpp"


class TimerChannelOnChip:public TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

    static bool isBasicTimer(const TIM_TypeDef * instance){
        #ifdef HAVE_TIM6
        if(instance == TIM6) return true;
        #endif
        #ifdef HAVE_TIM7
        if(instance == TIM7) return true;
        #endif
        return false;
    }

    static bool isGenericTimer(const TIM_TypeDef * instance){
        #ifdef HAVE_TIM2
        if(instance == TIM2) return true;
        #endif
        #ifdef HAVE_TIM3
        if(instance == TIM3) return true;
        #endif
        #ifdef HAVE_TIM4
        if(instance == TIM4) return true;
        #endif
        #ifdef HAVE_TIM5
        if(instance == TIM5) return true;
        #endif
        return false;
    }

    static bool isAdvancedTimer(const TIM_TypeDef * instance){
        #ifdef HAVE_TIM1
        if(instance == TIM1) return true;
        #endif
        #ifdef HAVE_TIM8
        if(instance == TIM8) return true;
        #endif
        #ifdef HAVE_TIM9
        if(instance == TIM9) return true;
        #endif
        #ifdef HAVE_TIM10
        if(instance == TIM10) return true;
        #endif
        return false;
    }
};
class TimerOutChannelOnChip:public TimerChannelOnChip{
public:

    enum class Mode:uint16_t{
        Timing = TIM_OCMode_Timing,
        Active = TIM_OCMode_Active,
        Inactive = TIM_OCMode_Inactive,
        Toggle = TIM_OCMode_Toggle,
        UpValid = TIM_OCMode_PWM1,
        DownValid = TIM_OCMode_PWM2
    };
protected:
    TIM_TypeDef * instance;
    const Channel channel;
    volatile uint16_t & from_channel_to_cvr(const Channel & _channel);
public:
    TimerOutChannelOnChip(TIM_TypeDef * _instance, const Channel _channel):
            instance(_instance), channel(_channel){;}
    void init(const bool & install = true, const Mode & mode = Mode::UpValid);

    void enable(const bool & en = true);

    void setMode(const Mode & _mode);
    void enableSync(const bool & _sync = true);
    void setPolarity(const bool & pol);
    void setIdleState(const bool & state);
    void installToPin(const bool & en = true);
};


class TimerOutChannelPosOnChip:public TimerOutChannelOnChip{
protected:
    volatile uint16_t & cvr;
    volatile uint16_t & arr;
public:
    TimerOutChannelPosOnChip(TIM_TypeDef * _base, const Channel & _channel);

    // __fast_inline uint16_t getPreloadData(){return instance->ATRLR;}

    __fast_inline TimerOutChannelPosOnChip & operator = (const int & _val){cvr = _val;return *this;}
    __fast_inline TimerOutChannelPosOnChip & operator = (const real_t & duty){cvr = int(duty * arr); return *this;}

    __fast_inline operator int(){return cvr;}
    __fast_inline operator real_t(){return real_t(cvr) / real_t(arr);}
    // TimerOutChannelOnChip & operator = (const bool en){enable(en); return *this;}
};

class TimerOutChannelNegOnChip:public TimerOutChannelOnChip{
public:
    TimerOutChannelNegOnChip(TIM_TypeDef * _base, const Channel _channel):TimerOutChannelOnChip(_base, _channel){;}
};
#endif