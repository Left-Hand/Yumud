#pragma once

#include "../hal/gpio/gpio.hpp"
#include "pwm_channel.hpp"

class GpioPwm:public PwmChannel, public Countable{
protected:
    Gpio & m_gpio;
public:
    volatile uint16_t m_cnt = 0;
    volatile uint16_t m_cvr = 0;
    volatile uint16_t m_arr = 1;
    GpioPwm(Gpio & gpio, const uint16_t arr):m_gpio(gpio), m_arr(arr){;}

    void init() override{m_gpio.outpp(false);}

    __fast_inline GpioPwm & operator = (const real_t duty) override{
        if(duty == 0) {m_cvr = 0;}
        else if(duty == 1) {m_cvr = m_arr - 1;}
        else {m_cvr = int(duty * m_arr);}
        return *this;
    }

    __fast_inline volatile uint16_t & cnt() override {return m_cnt;}
    __fast_inline volatile uint16_t & cvr() override {return m_cvr;}
    __fast_inline volatile uint16_t & arr() override {return m_arr;}

    __fast_inline void tick(){m_gpio = bool(m_cnt < m_cvr); m_cnt = (m_cnt + 1)% m_arr;}
    __fast_inline operator real_t(){return real_t(m_cvr) / real_t(m_arr);}
};