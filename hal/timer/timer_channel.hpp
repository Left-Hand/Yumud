#ifndef __TIMER_CHANNEL_HPP__

#define __TIMER_CHANNEL_HPP__

#include "sys/platform.h"
#include "pwm_channel.hpp"

class TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

protected:
    TIM_TypeDef * instance;
    const Channel channel;

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

    TimerChannel(TIM_TypeDef * _instance, const Channel _channel):instance(_instance), channel(_channel){;}
};



#endif