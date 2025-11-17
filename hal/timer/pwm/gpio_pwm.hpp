#pragma once

#include "hal/gpio/gpio_intf.hpp"
#include "primitive/pwm_channel.hpp"


namespace ymd::hal{
class GpioPwm final:
    public PwmIntf, 
    public CountableIntf<uint16_t>
{

public:

    GpioPwm(GpioIntf & gpio):gpio_(gpio){;}


    void init(const uint16_t _arr){
        gpio_.outpp(LOW);
        
        cnt_ = 0;
        arr_ = _arr;
    }

    __fast_inline volatile uint16_t & cnt() override {return cnt_;}
    __fast_inline volatile uint16_t & cvr() override {return cvr_;}
    __fast_inline volatile uint16_t & arr() override {return arr_;}
    
    __fast_inline void tick(){gpio_ = BoolLevel::from(cnt_ < cvr_); cnt_ = (cnt_ + 1)% arr_;}

    __inline void set_threshold(const uint16_t val){cvr_ = val - 1;}
    __inline void set_period(const uint16_t val){arr_ = val - 1;}

    void set_dutycycle(const real_t duty) override{
        if(duty == real_t(0)) {cvr_ = 0;}
        else if(duty == real_t(1)) {cvr_ = arr_ - 1;}
        else {cvr_ = int(duty * arr_);}
    }

    [[nodiscard]] real_t get_dutyscale(){return real_t(cvr_) / real_t(arr_);}
private:
    GpioIntf & gpio_;
    volatile uint16_t cnt_ = 0;
    volatile uint16_t cvr_ = 0;
    volatile uint16_t arr_ = 1;
};

}