#pragma once

#include "hal/gpio/gpio_intf.hpp"

namespace ymd::drivers{
class JQ8900{
public: 
    class Phy{
    public:
        Phy(hal::GpioIntf & ser):ser_(ser){}
        void tick();
        bool pending();
        void write(const uint8_t data);

        void init(){
            ser_.outpp();
        }
    private:
        hal::GpioIntf & ser_;
    };
private:
    Phy phy_;

    enum class Command:uint8_t{
        Clear = 0x0A,
        SetDisc = 0x0B,
        SetVolume = 0x0C,
        SetEQ = 0x0D,
    };

    void send_line(const uint8_t value, const Command cmd);
    void send_num(const int value);
public:
    JQ8900(hal::GpioIntf & ser):phy_(ser){};

    void init(){
        phy_.init();
    }
    void set_volume(const uint8_t value){send_line(value, Command::SetVolume);}
    void play_disc(const uint8_t value){send_line(value, Command::SetDisc);}

};

}