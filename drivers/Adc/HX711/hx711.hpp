#pragma once


#include "core/math/real.hpp"
#include "hal/gpio/gpio_intf.hpp"


namespace ymd::hal{
    class GpioIntf;
};

namespace ymd::drivers{

class HX711{
public:
    enum class ConvType{
        A128 = 1, B32, A64
    };
protected:
    hal::GpioIntf & sck_gpio_;
    hal::GpioIntf & sdo_gpio_;
    ConvType conv_type = ConvType::A128;

    uint32_t last_data;
    uint32_t zero_offset;
    bool inversed = false;

    static constexpr real_t GRAVITY_G = real_t(9.8);

    uint32_t read_data(void);

public:
    HX711(hal::GpioIntf & sck_gpio, hal::GpioIntf & sdo_gpio):
        sck_gpio_(sck_gpio), sdo_gpio_(sdo_gpio){;}
    ~HX711(){;}
    void init();
    bool is_idle();

    void update(){
        if(is_idle()) last_data = read_data();
    }

    void inverse(const Enable en = EN){
        inversed = en == EN;
    }
    void compensate(){
        while(!is_idle());
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
        return (GRAVITY_G * real_t(getWeightGram())) / 1000;
    }

    void setConvType(const ConvType & _convtype){
        conv_type = _convtype;
    }

};
}