
#pragma once

#include "Encoder.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/exti/exti.hpp"
#include "hal/timer/timer.hpp"

namespace ymd::drivers{

// class ABEncoderIntf:public AbsoluteEncoder{
// protected:
//     // real_t scale;
//     uint16_t cnt = 0;
// public:
//     // virtual uint16_t update() = 0;
// //     virtual real_t getLapPositionPerUnit() = 0;
// // public:
// //     constexpr ABEncoder(const uint16_t lines):
// //         scale(4.0 * lines / 65536){;}

// //     real_t getLapPosition(){
// //         return getLapPositionPerUnit() * scale;
// //     }
// };

// class ABEncoderTimer:public ABEncoderIntf{
// protected:
//     GenericTimer & inst;
//     const uint lines = 1 << 4;
// public:
//     ABEncoderTimer(GenericTimer & _inst):inst(_inst){;}
//     void init() override{
//         inst.initAsEncoder();
//     }

//     real_t getLapPosition() override {
//         real_t ret;
//         u16_to_uni(inst.cnt() * lines, ret);
//         return ret;
//     }

//     bool stable() const override{
//         return true;
//     }
// };

// class ABEncoderExti:public ABEncoderIntf{
// protected:
//     Gpio & trigGpioA;
//     Gpio & trigGpioB;

//     ExtiChannel trigExtiCHA;
//     ExtiChannel trigExtiCHB;

//     void a_pulse(){
//         if(bool(trigGpioA)){
//             if(bool(trigGpioB)) cnt--;
//             else cnt++;
//         }else{
//             if(bool(trigGpioB)) cnt++;
//             else cnt--;
//         }
//     };

//     void b_pulse(){
//         if(bool(trigGpioB)){
//             if(bool(trigGpioA)) cnt++;
//             else cnt--;
//         }else{
//             if(bool(trigGpioA)) cnt--;
//             else cnt++;
//         }
//     }
// public:
//     ABEncoderExti(Gpio & _trigGpioA, Gpio & _trigGpioB, const NvicPriority & _priority):
//         trigGpioA(_trigGpioA),
//         trigGpioB(_trigGpioB),
//         trigExtiCHA(trigGpioA, _priority, ExtiChannel::Trigger::RisingFalling),
//         trigExtiCHB(trigGpioB, _priority, ExtiChannel::Trigger::RisingFalling){;}

//     void init() override{
//         trigGpioA.inpu();
//         trigGpioB.inpu();
//         trigExtiCHA.bindCb(std::bind(&ABEncoderExti::a_pulse, this));
//         trigExtiCHB.bindCb(std::bind(&ABEncoderExti::b_pulse, this));
//         trigExtiCHA.init();
//         trigExtiCHB.init();
//     }
//     real_t getLapPosition() override{
//         real_t ret;
//         s16_to_uni(cnt, ret);
//         return ret;
//     }

// };


// class ABZEncoder:public ABEncoderIntf{

// };



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

class AbEncoderByGpio final{
public: 

    struct Config{
        Some<hal::Gpio *> a_gpio; 
        Some<hal::Gpio *> b_gpio;
    };

    AbEncoderByGpio(const Config & cfg):
        a_pin_(cfg.a_gpio.deref()), 
        b_pin_(cfg.b_gpio.deref()){;}

    void init(){
        a_pin_.inpu();
        b_pin_.inpu();    
    }

    [[nodiscard]] uint8_t get_code() const {

        const auto a = a_pin_.read().to_bool(); 
        const auto b = b_pin_.read().to_bool(); 

        return uint8_t(uint8_t(b) << 1) | uint8_t(a);
    }

    [[nodiscard]] constexpr int32_t count() const{
        return cnt_;
    }

    [[nodiscard]] constexpr uint32_t get_err_cnt() const{
        return err_cnt_;
    }

    void tick(){

        const auto this_code = get_code();

        if(last_code_ == UNSET){
            last_code_ = this_code;
            return;
        }
        const auto inc_code = INC_TABLE[last_code_];
        const auto dec_code = DEC_TABLE[last_code_];

        if(this_code == inc_code) {cnt_++;}
        else if(this_code == dec_code) {cnt_--;}
        else if(this_code != last_code_){err_cnt_++;}
        last_code_ = this_code;
    }

private:
    static constexpr uint8_t UNSET = 0xFF;

    static constexpr std::array<uint8_t, 4> INC_TABLE = {
        0b01,//00
        0b11,//01
        0b00,//10
        0b10 //11
    };

    static constexpr std::array<uint8_t, 4> DEC_TABLE = {
        0b10,//00
        0b00,//01
        0b11,//10
        0b01 //11
    };

    hal::Gpio & a_pin_;
    hal::Gpio & b_pin_;
    int32_t cnt_ = 0;
    uint32_t err_cnt_ = 0;
    uint8_t last_code_ = UNSET;
};

}