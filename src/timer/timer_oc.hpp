#ifndef __TIMER_OC_HPP__
#define __TIMER_OC_HPP__

#include "timer_channel.hpp"
#include "src/gpio/gpio.hpp"
#include "real.hpp"

class TimerOC:public TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

    enum class Mode:uint16_t{
        Timing = TIM_OCMode_Timing,
        Active = TIM_OCMode_Active,
        Inactive = TIM_OCMode_Inactive,
        Toggle = TIM_OCMode_Toggle,
        UpValid = TIM_OCMode_PWM1,
        DownValid = TIM_OCMode_PWM2
    };

protected:
    TIM_TypeDef * base;
    volatile uint16_t & cvr;

    const Channel channel;
    volatile uint16_t & from_channel_to_cvr(const Channel _channel);
public:


    TimerOC(TIM_TypeDef * _base, const Channel _channel);

    void init(const bool install = true, const Mode mode = Mode::UpValid);

    void enable(const bool en = true);

    void configSync(const bool _sync);
    void setPolarity(const bool pol);
    void setIdleState(const bool state);

    void setDuty(const real_t & duty){
        *this = (uint16_t)(int)(duty * base->CNT);
    }


    void installToPin(const bool en = true);

    TimerOC & operator = (const int _val){cvr = _val;return *this;}
    operator int(){return cvr;}
    TimerOC & operator = (const bool en){enable(en); return *this;}

};
#endif