#pragma once

#include "hal/gpio/gpio.hpp"

class HX711{
public:
    enum class ConvType{
        A128 = 1, B32, A64
    };
protected:
    GpioConcept & sck_pin;
    GpioConcept & sdo_pin;
    ConvType conv_type = ConvType::A128;

    uint32_t last_data;
    uint32_t zero_offset;
    bool inversed = false;

    scexpr real_t G = real_t(9.8);
    // scexpr real_t k = real_t(9.8 / 1000);

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
        sck_pin.outpp();
        sdo_pin.inpu();
        read_data();
    }

    bool isIdle(){
        return sdo_pin == false;
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
        return inversed ? (last_data - zero_offset) : (zero_offset - last_data);
    }

    int getWeightGram(){
        return getWeightData() / 1000;
    }

    real_t getNewton(){
        return (G * real_t(getWeightGram())) / 1000;
        // return real_t(getWeightGram());
    }

    void setConvType(const ConvType & _convtype){
        conv_type = _convtype;
    }

};
