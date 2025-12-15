#pragma once


#include "core/math/real.hpp"
#include "hal/gpio/gpio_intf.hpp"


namespace ymd::hal{
    class GpioIntf;
};

namespace ymd::drivers{

class HX711 final{
public:
    enum class ConvType{
        A128 = 1, B32, A64
    };
    static constexpr real_t GRAVITY_G = real_t(9.8);
public:
    explicit HX711(
        hal::GpioIntf & sck_gpio, 
        hal::GpioIntf & sdo_gpio
    ):
        sck_pin_(sck_gpio), 
        sdo_pin_(sdo_gpio){;}
    ~HX711(){;}
    void init();
    bool is_idle();

    void update(){
        if(is_idle()) last_data = read_data();
    }

    void inverse(const Enable en){
        inversed = en == EN;
    }
    void compensate(){
        while(!is_idle());
        update();
        zero_offset = last_data;
    }

    int32_t get_weight_data(){
        return inversed ? int32_t(last_data - zero_offset) : int32_t(zero_offset - last_data);
    }


    void set_conv_type(const ConvType & convtype){
        conv_type = convtype;
    }
private:
    hal::GpioIntf & sck_pin_;
    hal::GpioIntf & sdo_pin_;
    ConvType conv_type = ConvType::A128;

    uint32_t last_data;
    uint32_t zero_offset;
    bool inversed = false;
    uint32_t read_data(void);
};
}