#ifndef __AB_ENCODER_HPP__

#define __AB_ENCODER_HPP__

#include "Encoder.hpp"
#include "../hal/gpio/gpio.hpp"
#include "../hal/exti/exti.hpp"

class ABEncoder:public AbsoluteEncoder{
protected:
    // real_t scale;
    uint16_t cnt = 0;

//     virtual real_t getLapPositionPerUnit() = 0;
// public:
//     constexpr ABEncoder(const uint16_t lines):
//         scale(4.0 * lines / 65536){;}

//     real_t getLapPosition(){
//         return getLapPositionPerUnit() * scale;
//     }
};

class ABEncoderExti:public ABEncoder{
protected:
    Gpio & trigGpioA;
    Gpio & trigGpioB;

    ExtiChannel trigExtiCHA;
    ExtiChannel trigExtiCHB;



    void a_pulse(){
        if(bool(trigGpioA)){
            if(bool(trigGpioB)) cnt--;
            else cnt++;
        }else{
            if(bool(trigGpioB)) cnt++;
            else cnt--;
        }
    };

    void b_pulse(){
        if(bool(trigGpioB)){
            if(bool(trigGpioA)) cnt++;
            else cnt--;
        }else{
            if(bool(trigGpioA)) cnt--;
            else cnt++;
        }
    }
public:
    ABEncoderExti(Gpio & _trigGpioA, Gpio & _trigGpioB, const NvicPriority & _priority):
        trigGpioA(_trigGpioA),
        trigGpioB(_trigGpioB),
        trigExtiCHA(trigGpioA, _priority, ExtiChannel::Trigger::RisingFalling),
        trigExtiCHB(trigGpioB, _priority, ExtiChannel::Trigger::RisingFalling){;}

    void init() override{
        trigGpioA.inpu();
        trigGpioB.inpu();
        trigExtiCHA.bindCb(std::bind(&ABEncoderExti::a_pulse, this));
        trigExtiCHB.bindCb(std::bind(&ABEncoderExti::b_pulse, this));
        trigExtiCHA.init();
        trigExtiCHB.init();
    }
    real_t getLapPosition() override{
        real_t ret;
        s16_to_uni(cnt, ret);
        return ret;
    }

};
class ABZEncoder:public ABEncoder{

};



    // trigGpioA.InPullUP();
    // trigGpioB.InPullUP();
    // auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(0, 0), ExtiChannel::Trigger::RisingFalling);
    // auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 4), ExtiChannel::Trigger::RisingFalling);

    // int16_t cnt = 0;
    // trigExtiCHA.bindCb([&cnt, &trigGpioA, &trigGpioB](){
    //     if(bool(trigGpioA)){
    //         if(bool(trigGpioB)) cnt--;
    //         else cnt++;
    //     }else{
    //         if(bool(trigGpioB)) cnt++;
    //         else cnt--;
    //     }
    // });

    // trigExtiCHB.bindCb([&cnt,&trigGpioA,  &trigGpioB](){
    //     if(bool(trigGpioB)){
    //         if(bool(trigGpioA)) cnt++;
    //         else cnt--;
    //     }else{
    //         if(bool(trigGpioA)) cnt--;
    //         else cnt++;
    //     }
    // });


    // trigExtiCHA.init();
    // trigExtiCHB.init();


#endif