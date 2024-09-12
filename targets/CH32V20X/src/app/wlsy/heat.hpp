#ifndef __HEAT_HPP__

#define __HEAT_HPP__

#include "wlsy_inc.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "buck.hpp"

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
    static constexpr real_t R_kOhms = 100;
    static constexpr real_t R0_kOhms = 10;

    uint8_t index;

    real_t last_temp = 0;

    real_t get_uniV(){
        uint16_t data;

        switch(index){
            case 0:
                data = ADC1->IDATAR1;
                break;
            case 1:
                data = ADC1->IDATAR2;
                break;
            default:
                data = 0;
                break;
        }

        return real_t(data) >> 12;
    }

public:

    NTC(const uint8_t _index):index(_index){;}

    void init() override{

    };

    void update() override{
        static constexpr real_t T0= real_t(273.15+25);
        static constexpr real_t Ka= real_t(273.15);
        real_t VR = get_uniV();
        real_t Rt_kOhms = (VR)/(1-VR) * R_kOhms;
        auto this_temp = real_t(B/(B/T0+log(Rt_kOhms/R0_kOhms))) - Ka + real_t(0.5);
        last_temp = (last_temp * 15 + this_temp) >> 4;
    };



    real_t getRes(){
        real_t VR = get_uniV();
        return VR/(1-VR) * R_kOhms;
    }

    real_t getTemp(){
        return last_temp;
    }
};

class Heater{
protected:
    Gpio & sw_gpio;
    TempSensor & temp_sensor;
    Buck & buck;


public:
    Heater(Gpio & _heat_gpio, TempSensor & _temp_sensor, Buck & _buck):sw_gpio(_heat_gpio), temp_sensor(_temp_sensor), buck(_buck){;}

    void init(){
        sw_gpio.outpp();
        setTargetWatt(0);
    }

    void run(){
        temp_sensor.update();
    }

    void setTargetWatt(const real_t watt){
        buck.setTargetWatt(watt);
        sw_gpio = watt > real_t(0.1);
    }

    void on(){sw_gpio.set();}
    void off(){sw_gpio.clr();}
};

}

#endif