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
        A128 = 1, B32 = 2, A64 = 3
    };
    static constexpr iq16 GRAVITY_G = iq16(9.8);
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
        if(is_idle()) last_bits_ = read_data();
    }

    void inverse(const Enable en){
        inversed = en == EN;
    }
    void compensate(){
        while(!is_idle());
        update();
        zero_offset_ = last_bits_;
    }

    int32_t get_weight_data(){
        return inversed ? int32_t(last_bits_ - zero_offset_) : int32_t(zero_offset_ - last_bits_);
    }


    void set_conv_type(const ConvType & convtype){
        conv_type = convtype;
    }
private:
    hal::GpioIntf & sck_pin_;
    hal::GpioIntf & sdo_pin_;
    ConvType conv_type = ConvType::A128;

    uint32_t last_bits_;
    uint32_t zero_offset_;
    bool inversed = false;
    uint32_t read_data(void);
};
}