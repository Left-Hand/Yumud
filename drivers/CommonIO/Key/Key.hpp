#ifndef __KEY_HPP__

#define __KEY_HPP__

#include "../../hal/gpio/gpio.hpp"

struct Key{
protected:
    using Level = GpioUtils::Level;
    GpioConcept & m_gpio;
    const Level level_= GpioUtils::LOW;
public:
    Key(GpioConcept & gpio, const Level _level):m_gpio(gpio), level_(_level){;}

    void init(){
        init(level_);
    }
    void init(const Level level){
        if(level){
            m_gpio.inpd();
        }else{
            m_gpio.inpu();
        }
    }

    operator bool()const{
        return m_gpio.read() == level_;
    }

    auto & io(){
        return m_gpio;
    }
};
#endif