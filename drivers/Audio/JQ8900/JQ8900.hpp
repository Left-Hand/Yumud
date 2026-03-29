#pragma once

#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{

class JQ8900_Transport{
public:
    JQ8900_Transport(hal::GpioIntf & ser):pin_(ser){}
    void tick();
    bool pending();
    void write_byte(const uint8_t data);

    void init(){
        pin_.outpp();
    }
private:
    hal::GpioIntf & pin_;
};


class JQ8900{
public: 

private:
    using Transport = JQ8900_Transport;
    Transport transport_;

    enum class Command:uint8_t{
        Clear = 0x0A,
        SetDisc = 0x0B,
        SetVolume = 0x0C,
        SetEQ = 0x0D,
    };

    void send_line(const uint8_t value, const Command cmd);
    void send_num(const int value);
public:
    JQ8900(hal::GpioIntf & ser):transport_(ser){};

    void init(){
        transport_.init();
    }
    void set_volume(const uint8_t value){send_line(value, Command::SetVolume);}
    void play_disc(const uint8_t value){send_line(value, Command::SetDisc);}

};

}