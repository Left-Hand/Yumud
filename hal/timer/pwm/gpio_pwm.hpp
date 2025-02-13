#pragma once

#include "hal/gpio/gpio.hpp"
#include "pwm_channel.hpp"


namespace ymd{
class GpioPwm:public PwmChannel, public Countable<uint16_t>{
protected:
    Gpio & gpio_;
    volatile uint16_t cnt_ = 0;
    volatile uint16_t cvr_ = 0;
    volatile uint16_t arr_ = 1;
public:

    GpioPwm(Gpio & _gpio):gpio_(_gpio){;}

    void init(){
        gpio_.outpp(LOW);
        cnt_ = 0;
    }

    void init(const uint16_t _arr){
        init();
        arr_ = _arr;
    }

    __fast_inline volatile uint16_t & cnt() override {return cnt_;}
    __fast_inline volatile uint16_t & cvr() override {return cvr_;}
    __fast_inline volatile uint16_t & arr() override {return arr_;}
    
    __fast_inline void tick(){gpio_ = bool(cnt_ < cvr_); cnt_ = (cnt_ + 1)% arr_;}

    __inline void setThreshold(const uint16_t val){cvr_ = val - 1;}
    __inline void setPeriod(const uint16_t val){arr_ = val - 1;}

    __fast_inline GpioPwm & operator = (const real_t duty) override{
        if(duty == 0) {cvr_ = 0;}
        else if(duty == 1) {cvr_ = arr_ - 1;}
        else {cvr_ = int(duty * arr_);}
        return *this;
    }

    __inline operator real_t(){return real_t(cvr_) / real_t(arr_);}
};

}