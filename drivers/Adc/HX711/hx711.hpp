#pragma once

#include "hal/gpio/gpio.hpp"
#include "sys/math/real.hpp"

namespace ymd::drivers{

class HX711{
public:
    enum class ConvType{
        A128 = 1, B32, A64
    };
protected:
    hal::Gpio & sck_gpio_;
    hal::Gpio & sdo_gpio_;
    ConvType conv_type = ConvType::A128;

    uint32_t last_data;
    uint32_t zero_offset;
    bool inversed = false;

    scexpr real_t G = real_t(9.8);

    uint32_t read_data(void);

    class HX711Channel{

    };
public:
    HX711(hal::Gpio & sck_gpio, hal::Gpio & sdo_gpio):
        sck_gpio_(sck_gpio), sdo_gpio_(sdo_gpio){;}
    ~HX711(){;}
    void init(){
        sck_gpio_.outpp();
        sdo_gpio_.inpu();
        read_data();
    }

    bool isIdle(){
        return sdo_gpio_ == false;
    }

    void update(){
        if(isIdle()) last_data = read_data();
    }

    void inverse(const bool en = true){
        inversed = en;
    }
    void compensate(){
        while(!isIdle());
        update();
        zero_offset = last_data;
    }

    int getWeightData(){
        return inversed ? int(last_data - zero_offset) : int(zero_offset - last_data);
    }

    int getWeightGram(){
        return getWeightData() / 1000;
    }

    real_t getNewton(){
        return (G * real_t(getWeightGram())) / 1000;
    }

    void setConvType(const ConvType & _convtype){
        conv_type = _convtype;
    }

};
}