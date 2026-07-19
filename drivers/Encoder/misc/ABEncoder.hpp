
#pragma once

#include "Encoder.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/exti/exti.hpp"
#include "hal/timer/timer.hpp"

namespace ymd::drivers{

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

    [[nodiscard]] uint8_t get_code() const noexcept {

        const auto a = a_pin_.read().to_bool(); 
        const auto b = b_pin_.read().to_bool(); 

        return uint8_t(uint8_t(b) << 1) | uint8_t(a);
    }

    [[nodiscard]] constexpr int32_t count() const noexcept {
        return cnt_;
    }

    [[nodiscard]] constexpr uint32_t get_err_cnt() const noexcept {
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