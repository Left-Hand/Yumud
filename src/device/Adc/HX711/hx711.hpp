#ifndef __HX711_HPP__

#define __HX711_HPP__

#include "src/gpio/gpio.hpp"

class HX711{
public:
    enum class ConvType{
        A128 = 1, B32, A64
    };
protected:
    const uint32_t err_data = 0xFFFFFFFF;
    GpioConcept & sck_pin;
    GpioConcept & sdo_pin;
    ConvType conv_type = ConvType::A128;

    uint32_t read_data(void){
        uint32_t data=0;

        for(uint8_t i = 0; i < 24; i++){
            sck_pin = true;
            __nopn(2);
            sck_pin = false;

            data <<= 1; data |= bool(sdo_pin);
        }

        for(uint8_t i = 0; i < (uint8_t)conv_type; i++){
            sck_pin = true;
            __nopn(2);
            sck_pin = false;
        }

        data ^= 0x800000;
        return(data);
    }
public:
    HX711(GpioConcept & _sck_pin, GpioConcept & _sdo_pin):sck_pin(_sck_pin), sdo_pin(_sdo_pin){;}
    ~HX711(){;}
    void init(){
        sck_pin.OutPP();
        sdo_pin.InPullUP();
        read_data();
    }

    bool isIdle(){
        return sdo_pin == false;
    }

    void getWeightData(uint32_t & weight){
        weight = read_data();
    }

    void setConvType(const ConvType & _convtype){
        conv_type = _convtype;
    }

};
#endif