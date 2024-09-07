#ifndef __HEAT_HPP__

#define __HEAT_HPP__

#include "wlsy_inc.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"

namespace WLSY{
class TempSensor{
public:
    virtual void init() = 0;
    virtual void update() = 0;
    virtual real_t getTemp() = 0;
};

class NTC:public TempSensor{
protected:
    static constexpr real_t B = 3950;
    static constexpr real_t R_kOhms = 51;
    static constexpr real_t R0_kOhms = 100;

    real_t get_uniV(){
        return real_t(uint16_t(ADC1->IDATAR1)) / real_t(4096);
    }

public:
    void init() override{

    };

    void update() override{

    };

    real_t getTemp(){
        
        static constexpr real_t T0= real_t(273.15+25);
        static constexpr real_t Ka= real_t(273.15);
        real_t VR = get_uniV();
        real_t Rt_kOhms = (1-VR)/VR * R_kOhms;
        return real_t(1/(1/T0+log(Rt_kOhms/R0_kOhms)/B)) - Ka + real_t(0.5);
    }
};

class Heater{
protected:
    Gpio & heat_gpio;
    TempSensor & temp_sensor;
    real_t target_temp;
    real_t ripple_temp = 5;

    void on(){heat_gpio.set();}

    void off(){heat_gpio.clr();}
public:
    Heater(Gpio & _heat_gpio, TempSensor & _temp_sensor):heat_gpio(_heat_gpio), temp_sensor(_temp_sensor){;}

    void init(){
        heat_gpio.outpp(0);
        setTargetTemp(50);
    }

    void run(){
        temp_sensor.update();
        on();
        // const real_t temp = temp_sensor.getTemp();
        // if(temp > target_temp + ripple_temp){
        //     off();
        // }else if(temp < target_temp - ripple_temp){
        //     on();
        // }
    }

    void setTargetTemp(const real_t temp){
        target_temp = temp;
    }
};

}

#endif