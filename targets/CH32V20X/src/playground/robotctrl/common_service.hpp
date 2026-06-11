#pragma once

#include "hal/gpio/gpio.hpp"
namespace ymd::robots{

class LedService{
public:
    enum class BreathMethod:uint8_t{
        Sine = 0,
        Saw,
        Square,
        Triangle
    };
protected:
    struct Blink{
        Milliseconds on;
        Milliseconds off;
        size_t times;
    };

    struct Pulse{
        Milliseconds on;
    };

public:

    void resume();

private:
    // void set_color(const Color<iq16> & color){

    // }
};

class BeepService{
public:
    struct Config{

    };

    explicit BeepService(
        Some<hal::Gpio *> gpio
    ): gpio_(gpio.deref()){;}

    void push_pulse(const Milliseconds period){

    };

    void resume(){

    }

private:
    hal::Gpio & gpio_;
};

}